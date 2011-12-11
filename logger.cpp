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
    qDebug() << "Logger::~Logger";
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

LoggerWorker::LoggerWorker(QObject *parent): QObject(parent), lastSteps(-1) {
    db = new Database(Platform::instance()->dbPath(), this);
    connect(db, SIGNAL(initialize()), this, SLOT(onInitialize()));
}

void LoggerWorker::log(int steps, const QVariantMap &tags) {
    archiveIfOld();
    insertLog(steps, tags);
}

void LoggerWorker::archiveIfOld() {
    if (QDate::currentDate() != Preferences::instance()->logDate()) {
        qDebug() << "LoggerWorker::archiveIsOld: Archiving needed";
        archive();
    }
}

bool LoggerWorker::insertLog(int steps, const QVariantMap &tags) {
    bool success = false;

    QDateTime now = QDateTime::currentDateTime();
    if ((lastSteps == steps) && !tags.size() && (lastDate.secsTo(now) < MinTimeDiff)) {
        return true;
    }
    lastSteps = steps;
    lastDate = now;

    if (!db->db().transaction()) {
        qCritical() << "LoggerWorker::insertLog: Can't start transaction:" << db->db().lastError().text();
        return false;
    }

    QSqlQuery query("insert into log (date, steps) values (?, ?)", db->db());
    query.bindValue(0, now.toString(Qt::ISODate));
    query.bindValue(1, steps);
    success = query.exec();
    if (success) {
        qlonglong id = query.lastInsertId().toLongLong();
        foreach (QString key, tags.keys()) {
            QString value = tags[key].toString();
            QSqlQuery tagQuery("insert into tags (name, value, logid) values (?, ?, ?)", db->db());
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
        db->db().commit();
    } else {
        qCritical() << "LoggerWorker::insertLog: Failed to log:" << db->db().lastError().text();
        db->db().rollback();
    }
    return success;
}

void LoggerWorker::archive() {
    qDebug() << "LoggerWorker::archive";
    db->db().close();
    QString archiveName = getArchiveName();
    if (!QFile().rename(Platform::instance()->dbPath(), archiveName)) {
        qCritical() << "LoggerWorker::archive: Failed to rename" << Platform::instance()->dbPath() << "to" << archiveName;
    }
}

void LoggerWorker::onInitialize() {
    qDebug() << "LoggerWorker::onInitialize";

    // Set database schema
    QSqlQuery query(db->db());
    if (!query.exec("create table if not exists log (id integer primary key, date varchar, steps integer)")) {
        qCritical() << "LoggerWorker::onInitialize: Failed to create log table:" << query.lastError().text();
        return;
    }
    if (!query.exec("create table if not exists tags (name varchar, value varchar, logid integer, foreign key(logid) references log(id))")) {
        qCritical() << "LoggerWorker::onInitialize: Failed to create tags table:" << query.lastError().text();
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
