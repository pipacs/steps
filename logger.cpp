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

/// Minimum time difference for logging unchanged information (seconds).
const int MinTimeDiff = 60 * 60;

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

LoggerWorker::LoggerWorker(QObject *parent): QObject(parent), lastSteps(-1), db_(0) {
}

LoggerWorker::~LoggerWorker() {
    delete db_;
}

Database *LoggerWorker::db() {
    if (!db_) {
        db_ = new Database(Platform::instance()->dbPath());
        connect(db_, SIGNAL(addSchema()), this, SLOT(onAddSchema()));
    }
    return db_;
}

void LoggerWorker::log(int steps, const QVariantMap &tags) {
    archiveIfOld();
    insertLog(steps, tags);
}

void LoggerWorker::archiveIfOld() {
    if (QDate::currentDate() != Preferences::instance()->logDate()) {
        archive();
    }
}

void LoggerWorker::insertLog(int steps, const QVariantMap &tags) {
    QDateTime now = QDateTime::currentDateTime();
    if ((lastSteps == steps) && !tags.size() && (lastDate.secsTo(now) < MinTimeDiff)) {
        return;
    }
    lastSteps = steps;
    lastDate = now;

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
        qlonglong id = query.lastInsertId().toLongLong();
        foreach (QString key, tags.keys()) {
            QString value = tags[key].toString();
            QSqlQuery tagQuery(db()->db());
            tagQuery.prepare("insert into tags (name, value, logid) values (?, ?, ?)");
            tagQuery.bindValue(0, key);
            tagQuery.bindValue(1, value);
            tagQuery.bindValue(2, id);
            if (!tagQuery.exec()) {
                success = false;
                break;
            }
        }
    }
    if (success) {
        db()->commit();
    } else {
        qCritical() << "LoggerWorker::insertLog: Failed to log:" << db()->error();
        db()->rollback();
    }
}

void LoggerWorker::archive() {
    Trace t("LoggerWorker::archive");
    QString dbName = Platform::instance()->dbPath();
    QFile file(dbName);
    if (!file.exists()) {
        qDebug() << "No database:" << dbName << "does not exist";
        return;
    }
    db()->close();
    QString archiveName = getArchiveName();
    if (!file.rename(archiveName)) {
        qCritical() << "LoggerWorker::archive: Error" << file.error() << ":" << file.errorString() << ": Failed to rename" << dbName << "to" << archiveName;
    } else {
        qDebug() << "Renamed" << dbName << "to" << archiveName;
    }
}

void LoggerWorker::onAddSchema() {
    // Set database schema
    QSqlQuery query(db()->db());
    if (!query.exec("create table log (id integer primary key, date varchar, steps integer)")) {
        qCritical() << "LoggerWorker::onAddSchema: Failed to create log table:" << query.lastError().text();
        return;
    }
    if (!query.exec("create table tags (name varchar, value varchar, logid integer, foreign key(logid) references log(id))")) {
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
    insertLog(-1, tags);
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
