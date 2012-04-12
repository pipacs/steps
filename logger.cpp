#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

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

void Logger::archive() {
    if (!QMetaObject::invokeMethod(worker, "archive")) {
        qCritical() << "Logger::archive: Invoking remote logger failed";
    }
}

LoggerWorker::LoggerWorker(QObject *parent): QObject(parent), database(0), logCount(0), diskFull(false), lastInsertId(0), totalSteps(0), lastLogTime(QDate(1965, 06, 29)) {
}

LoggerWorker::~LoggerWorker() {
    delete database;
}

Database *LoggerWorker::db() {
    if (!database) {
        database = new Database(Platform::instance()->dbPath());
        connect(database, SIGNAL(addSchema()), this, SLOT(onAddSchema()), Qt::QueuedConnection);
    }
    return database;
}

void LoggerWorker::log(int steps, const QVariantMap &tags) {
    archiveIfOld();
    saveLog(steps, tags);
}

void LoggerWorker::archiveIfOld() {
    if (QDate::currentDate() != Preferences::instance()->logDate()) {
        archive();
    }
}

void LoggerWorker::saveLog(int steps, const QVariantMap &tags) {
    // Return if nothing to log
    if ((steps == 0) && !tags.size()) {
        return;
    }

    // Check disk space every now and then
    if ((logCount++ % 50) == 0) {
        diskFull = Platform::instance()->dbFull();
        if (diskFull) {
            qCritical() << "LoggerWorker::saveLog: Disk full";
        }
    }

    // Don't log if the disk is full
    if (diskFull) {
        return;
    }

    // Insert new record or update the last one
    QDateTime now = QDateTime::currentDateTime();
    qint64 elapsed = now.toMSecsSinceEpoch() - lastLogTime.toMSecsSinceEpoch();
    if (!lastInsertId || tags.count() || (elapsed > MinInsertTimeDiff)) {
        insertLog(now, steps, tags);
    } else {
        updateLog(now, steps);
    }

    // If there were tags to log, force a new insert next time
    if (tags.count()) {
        lastInsertId = 0;
    }

    lastLogTime = now;
}

void LoggerWorker::updateLog(const QDateTime &now, int steps) {
    QSqlQuery query(db()->db());
    query.prepare("update log set date=?, steps=? where id=?");
    query.bindValue(0, now.toString(Qt::ISODate));
    query.bindValue(1, totalSteps + steps);
    query.bindValue(2, lastInsertId);
    if (!query.exec()) {
        qCritical() << "LoggerWorker::updateLog: Failed to log:" << db()->error();
    } else {
        totalSteps += steps;
    }
}

void LoggerWorker::insertLog(const QDateTime &now, int steps, const QVariantMap &tags) {
    lastInsertId = 0;
    totalSteps = 0;

    if (!db()->transaction()) {
        qCritical() << "LoggerWorker::insertLog: Can't start transaction:" << db()->error();
        return;
    }

    QSqlQuery query(db()->db());
    query.prepare("insert into log (date, steps) values (?, ?)");
    query.bindValue(0, now.toString(Qt::ISODate));
    query.bindValue(1, steps);
    bool success = query.exec();
    if (success) {
        lastInsertId = query.lastInsertId().toLongLong();
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
        totalSteps = steps;
    } else {
        db()->rollback();
        lastInsertId = 0;
    }
}

void LoggerWorker::archive() {
    QString dbName = Platform::instance()->dbPath();
    QFile file(dbName);
    if (!file.exists()) {
        qWarning() << "LoggerWorker::archive: No database:" << dbName << "does not exist";
        return;
    }
    db()->close();
    lastInsertId = 0;
    QString archiveName = getArchiveName();
    if (!file.rename(archiveName)) {
        qCritical() << "LoggerWorker::archive: Error" << file.error() << ":" << file.errorString() << ": Failed to rename" << dbName << "to" << archiveName;
    }
}

void LoggerWorker::onAddSchema() {
    // Set database schema
    QSqlQuery query(db()->db());
    if (!query.exec("create table log (id integer primary key, date varchar, steps integer);")) {
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
    saveLog(0, tags);
}

QString LoggerWorker::getArchiveName() {
    QString dir = QFileInfo(Platform::instance()->dbPath()).absolutePath();
    QString base = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    QString archiveName = dir + "/" + base + ".adb";
    if (QFileInfo(archiveName).exists()) {
        qCritical() << "LoggerWorker::getArchiveName: Archive" << archiveName << "exists already";
        return QString();
    }
    return archiveName;
}
