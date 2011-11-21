#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include "logger.h"
#include "platform.h"

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
    worker = new LoggerWorker(this);
    workerThread = new QThread(this);
    worker->moveToThread(workerThread);
    workerThread->start(QThread::LowestPriority);
}

Logger::~Logger() {
    workerThread->quit();
    workerThread->wait();
}

void Logger::log(int steps, const QVariantHash &tags) {
    if (!QMetaObject::invokeMethod(
            worker,
            "doLog",
            Q_ARG(int, steps),
            Q_ARG(QVariantHash, tags))) {
        qCritical() << "Logger::log: Invoking remote logger failed";
    }
}

LoggerWorker::LoggerWorker(QObject *parent): QObject(parent), lastSteps(-1) {
    bool shouldCreate = false;
    QFileInfo info(Platform::instance()->dbPath());
    if (!info.exists()) {
        (void)QDir().mkpath(info.absolutePath());
        shouldCreate = true;
    }
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::toNativeSeparators(info.absoluteFilePath()));
    if (!db.open()) {
        qCritical() << "LoggerWorker::LoggerWorker: Could not open" << Platform::instance()->dbPath() << ": Error" << db.lastError().text();
        return;
    }
    if (shouldCreate) {
        create();
    }
}

void LoggerWorker::create() {
    QSqlQuery query;
    if (!query.exec("create table log (id integer primary key, date varchar, steps integer)")) {
        qCritical() << "LoggerWorker::LoggerWorker: Failed to create log table:" << query.lastError().text();
        return;
    }
    if (!query.exec("create table tags (name varchar, value varchar, logid integer, foreign key(logid) references log(id))")) {
        qCritical() << "LoggerWorker::LoggerWorker: Failed to create tags table:" << query.lastError().text();
    }
}

LoggerWorker::~LoggerWorker() {
    db.close();
}

void LoggerWorker::doLog(int steps, const QVariantHash &tags) {
    bool success;
    QDateTime now = QDateTime::currentDateTimeUtc();

    if ((lastSteps == steps) && (lastTags == tags) && (lastDate.secsTo(now) < MinTimeDiff)) {
        return;
    }

    lastSteps = steps;
    lastDate = now;
    lastTags = tags;

    if (!db.transaction()) {
        qCritical() << "LoggerWorker::doLog: Can't start transaction:" << db.lastError().text();
        return;
    }

    QSqlQuery query("insert into log (date, steps) values (?, ?)");
    query.bindValue(0, now.toString(Qt::ISODate));
    query.bindValue(1, steps);
    success = query.exec();
    if (success) {
        qlonglong id = query.lastInsertId().toLongLong();
        foreach (QString key, tags.keys()) {
            QString value = tags[key].toString();
            QSqlQuery tagQuery("insert into tags (name, value, logid) values (?, ?, ?)");
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
        db.commit();
    } else {
        qCritical() << "LoggerWorker::doLog: Failed to log:" << db.lastError().text();
        db.rollback();
    }
}
