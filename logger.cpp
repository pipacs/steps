#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include <sqlite3.h>

#include "logger.h"
#include "platform.h"
#include "preferences.h"
#include "database.h"
#include "trace.h"

/// Minimum time difference to force logging a new record (otherwise the last record gets updated; ms)
const qint64 MinInsertTimeDiff = 5 * 60 * 1000;

Logger *instance_;

Logger *Logger::instance() {
    if (!instance_) {
        instance_ = new Logger();
    }
    return instance_;
}

void Logger::close() {
    delete instance_;
    instance_ = 0;
}

Logger::Logger(QObject *parent): QObject(parent) {
    worker = new LoggerWorker();
    workerThread = new QThread(this);
    worker->moveToThread(workerThread);
    workerThread->start(QThread::LowestPriority);
}

Logger::~Logger() {
    workerThread->quit();
    workerThread->wait();
    delete worker;
}

void Logger::log(int steps, const QVariantMap &tags) {
    if (!QMetaObject::invokeMethod(
            worker,
            "log",
            Q_ARG(int, steps),
            Q_ARG(QVariantMap, tags))) {
        qCritical() << "Logger::log: Invoking remote logger failed";
    }
}

void Logger::upgrade() {
    if (!QMetaObject::invokeMethod(worker, "upgrade")) {
        qCritical() << "Logger::upgrade: Invoking remote logger failed";
    }
}

LoggerWorker::LoggerWorker(QObject *parent): QObject(parent), database(0), logCount(0), diskFull(false) {
}

LoggerWorker::~LoggerWorker() {
    delete database;
}

Database *LoggerWorker::db() {
    if (!database) {
        database = new Database(Platform::instance()->dbPath());
        connect(database, SIGNAL(addSchema()), this, SLOT(onAddSchema()));
    }
    return database;
}

void LoggerWorker::log(int steps, const QVariantMap &tags) {
    Trace _("LoggerWorker::log");

    // Return if nothing to log
    if ((steps == 0) && !tags.size()) {
        qDebug() << "Nothing to do";
        return;
    }

    // Check disk space every now and then
    if ((logCount++ % 500) == 0) {
        diskFull = Platform::instance()->dbFull();
        if (diskFull) {
            qCritical() << "LoggerWorker::log: Disk full";
        }
    }

    // Don't log if the disk is full
    if (diskFull) {
        return;
    }

    // Insert new record
    insertLog(steps, tags);
}

void LoggerWorker::insertLog(int steps, const QVariantMap &tags) {
    Trace _("LoggerWorker::insertLog");

    if (!db()->transaction()) {
        qCritical() << "LoggerWorker::insertLog: Can't start transaction:" << db()->error();
        return;
    }

    QDateTime now = QDateTime::currentDateTimeUtc();
    QSqlQuery query(db()->db());
    query.prepare("insert into log (date, steps, inqc, ingft) values (?, ?, 0, 0)");
    query.bindValue(0, now.toString(Qt::ISODate));
    query.bindValue(1, steps);
    bool success = query.exec();
    if (success) {
        qlonglong lastInsertId = query.lastInsertId().toLongLong();
        foreach (QString key, tags.keys()) {
            QString value = tags[key].toString();
            QSqlQuery tagQuery(db()->db());
            tagQuery.prepare("insert into tags (name, value, logid) values (?, ?, ?)");
            tagQuery.bindValue(0, key);
            tagQuery.bindValue(1, value);
            tagQuery.bindValue(2, lastInsertId);
            if (!tagQuery.exec()) {
                success = false;
                qCritical() << "LoggerWorker::insertLog: Inserting tag" << key << value << "failed:" << db()->error();
                break;
            }
        }
    } else {
        qCritical() << "LoggerWorker::insertLog: Inserting log failed:" << db()->error();
    }

    if (success) {
        db()->commit();
    } else {
        db()->rollback();
    }
}

void LoggerWorker::onAddSchema() {
    Trace _("LoggerWorker::onAddSchema");

    // Set database schema
    QSqlQuery query(db()->db());
    // if (!query.exec("create table log (id integer primary key, date varchar, steps integer);")) {
    if (!query.exec("create table log (id integer primary key, date varchar, steps integer, inqc integer, ingft integer);")) {
        qCritical() << "LoggerWorker::onAddSchema: Failed to create log table:" << query.lastError().text();
        return;
    }
    query.clear();
    if (!query.exec("create table tags (name varchar, value varchar, logid integer, foreign key(logid) references log(id));")) {
        qCritical() << "LoggerWorker::onAddSchema: Failed to create tags table:" << query.lastError().text();
        return;
    }

    // Save date of initialization to settings
    Preferences::instance()->setLogDate(QDate::currentDate());

    // Record the current time zone (as seconds from UTC)
    QVariantMap tags;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nowUtc(now);
    nowUtc.setTimeSpec(Qt::UTC);
    tags.insert("secondsFromUtc", now.secsTo(nowUtc));
    log(0, tags);
}

void LoggerWorker::upgrade() {
    Trace _("LoggerWorker::upgrade");
    QString dbName = Platform::instance()->dbPath();
    QString dirName = QFileInfo(dbName).absolutePath();

    // Upgrade current log
    if (QFileInfo(dirName + "/current.db").exists()) {
        upgradeDbToDc(dirName + "/current.db");
    }

    // Delete old archives
    QDir dir(dirName);
    QStringList nameList;
    nameList << "*.adb";
    nameList << "*.adc";
    foreach (QString srcName, dir.entryList(nameList, QDir::Files | QDir::Readable)) {
        upgradeDbToDc(dirName + "/" + srcName);
    }
}

void LoggerWorker::upgradeDbToDc(const QString &srcName) {
    Trace _("LoggerWorker::upgradeDbToDc");

    qDebug() << srcName << "->" << Platform::instance()->dbPath();
    Database srcDb(srcName);
    Database dstDb(Platform::instance()->dbPath());

    QSqlQuery dstQuery(dstDb.db());
    QSqlQuery srcQuery(srcDb.db());
    QSqlQuery srcTagsQuery(srcDb.db());
    QSqlQuery dstTagsQuery(dstDb.db());

    QStringList interestingTags;
    interestingTags << "appVersion";
    interestingTags << "detectedActivity";
    interestingTags << "osName";
    interestingTags << "secondsFromUtc";

    QString lastActivity;

    // Initialize upgraded database

    if (!dstQuery.exec("create table if not exists log (id integer primary key, date varchar, steps integer, inqc integer, ingft integer);")) {
        qCritical() << "LoggerWorker::upgradeDbToDc: Failed to create log table:" << dstQuery.lastError().text();
        return;
    }
    dstQuery.clear();
    if (!dstQuery.exec("create table if not exists tags (name varchar, value varchar, logid integer, foreign key(logid) references log(id));")) {
        qCritical() << "LoggerWorker::upgradeDbToDc: Failed to create tags table:" << dstQuery.lastError().text();
        return;
    }

    // Read log records

    if (!srcQuery.exec("select id, date, steps from log")) {
        qCritical() << "LoggerWorker::upgradeDbToDc: Could not query source:" << srcQuery.lastError().text();
        return;
    }

    while (srcQuery.next()) {
        qlonglong id = srcQuery.value(0).toLongLong();
        QString date = srcQuery.value(1).toString();
        int steps = srcQuery.value(2).toInt();

        // Read tags
        QMap<QString, QString> tags;
        srcTagsQuery.clear();
        srcTagsQuery.prepare("select name, value from tags where logId = ?");
        srcTagsQuery.bindValue(0, id);
        if (!srcTagsQuery.exec()) {
            qCritical() << "LoggerWorker::upgradeDbToDc: Could not query tags:" << srcTagsQuery.lastError().text();
            continue;
        }
        while (srcTagsQuery.next()) {
            QString name = srcTagsQuery.value(0).toString();
            QString value = srcTagsQuery.value(1).toString();

            // Filter out activity flip-flopping
            if (name == "detectedActivity") {
                if ((value == "idle") || (value == lastActivity)) {
                    continue;
                }
                lastActivity = value;
            }

            // Filter out non-interesting tags
            if (interestingTags.contains(name)) {
                tags.insert(name, value);
            }
        }

        // Escape if there is no step count nor tags
        if ((steps == 0) && tags.isEmpty()) {
            continue;
        }

        // Save upgraded log record
        dstQuery.clear();
        dstQuery.prepare("insert into log (date, steps, inqc, ingft) values (?, ?, 0, 0)");
        dstQuery.bindValue(0, date);
        dstQuery.bindValue(1, steps);
        if (!dstQuery.exec()) {
            qCritical() << "LoggerWorker::upgradeDbToDc: Upgrading record" << id << "failed:" << dstTagsQuery.lastError().text();
            continue;
        }

        // Save tags
        qlonglong lastInsertId = dstQuery.lastInsertId().toLongLong();
        foreach (QString key, tags.keys()) {
            QString value = tags.value(key);
            dstTagsQuery.clear();
            dstTagsQuery.prepare("insert into tags (name, value, logid) values (?, ?, ?)");
            dstTagsQuery.bindValue(0, key);
            dstTagsQuery.bindValue(1, value);
            dstTagsQuery.bindValue(2, lastInsertId);
            if (!dstTagsQuery.exec()) {
                qCritical() << "LoggerWorker::upgradeDbToDc: Inserting tag" << key << value << "failed:" << dstTagsQuery.lastError().text();
                break;
            }
        }
    }

    QFile(srcName).remove();
}
