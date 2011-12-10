#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include "logger.h"
#include "platform.h"
#include "preferences.h"

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
    bool shouldCreate = false;
    QFileInfo info(Platform::instance()->dbPath());
    if (!info.exists()) {
        (void)QDir().mkpath(info.absolutePath());
        shouldCreate = true;
    }
    db = QSqlDatabase::addDatabase("QSQLITE");
}

LoggerWorker::~LoggerWorker() {
    qDebug() << "LoggerWorker::~LoggerWorker";
    db.close();
}

void LoggerWorker::log(int steps, const QVariantMap &tags) {
    if (checkDb(steps)) {
        insertLog(steps, tags);
    }
}

bool LoggerWorker::insertLog(int steps, const QVariantMap &tags) {
    bool success = false;

    QDateTime now = QDateTime::currentDateTime();
    if ((lastSteps == steps) && !tags.size() && (lastDate.secsTo(now) < MinTimeDiff)) {
        qDebug() << " Nothing new to log";
        return true;
    }
    lastSteps = steps;
    lastDate = now;

    if (!db.transaction()) {
        qCritical() << "LoggerWorker::insertLog: Can't start transaction:" << db.lastError().text();
        return false;
    }

    qDebug() << "LoggerWorker::insertLog:" << now.toString(Qt::ISODate) << ":" << steps;
    QSqlQuery query("insert into log (date, steps) values (?, ?)", db);
    query.bindValue(0, now.toString(Qt::ISODate));
    query.bindValue(1, steps);
    success = query.exec();
    if (success) {
        qlonglong id = query.lastInsertId().toLongLong();
        foreach (QString key, tags.keys()) {
            QString value = tags[key].toString();
            qDebug() << "" << key << ":" << value;
            QSqlQuery tagQuery("insert into tags (name, value, logid) values (?, ?, ?)", db);
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
        qCritical() << "LoggerWorker::insertLog: Failed to log:" << db.lastError().text();
        db.rollback();
    }
    return success;
}

bool LoggerWorker::checkDb(int steps) {
    // Open database if not opened yet
    if (!db.isOpen()) {
        QString dbPath = Platform::instance()->dbPath();
        QFileInfo dbInfo(dbPath);
        if (!dbInfo.exists()) {
            // Create database if file doesn't exist
            return create(steps);
        }
        // Otherwise open database with the existing file
        db.setDatabaseName(QDir::toNativeSeparators(dbPath));
        if (!db.open()) {
            qCritical() << "LoggerWorker::checkDb: Could not open database";
            return false;
        }
    }

    // Archive database if it's not from today
    QDate today = QDate::currentDate();
    if (today != Preferences::instance()->logDate()) {
        return archive(steps);
    }

    // Else we are current
    return true;
}

bool LoggerWorker::archive(int steps) {
    qDebug() << "LoggerWorker::archive";
    if (!db.isOpen()) {
        qCritical() << "LoggerWorker::archive: Database not open";
        return false;
    }

    db.close();
    QString archiveName = getArchiveName();
    if (!QFile().rename(Platform::instance()->dbPath(), archiveName)) {
        qCritical() << "LoggerWorker::archive: Failed to rename" << Platform::instance()->dbPath() << "to" << archiveName;
        return false;
    }

    return create(steps);
}

bool LoggerWorker::create(int steps) {
    qDebug() << "LoggerWorker::create";
    QString dbPath =  Platform::instance()->dbPath();
    QFileInfo dbInfo(dbPath);
    QString dir = dbInfo.absolutePath();
    QFileInfo dirInfo(dir);

    if (!dirInfo.exists()) {
        if (!QDir().mkpath(dir)) {
            qCritical() << "LoggerWorker::create: Failed to create" << dir;
            return false;
        }
    } else if (!dirInfo.isDir()) {
        qCritical() << "LoggerWorker::create:" << dir << "is not a directory";
        return false;
    }

    db.setDatabaseName(QDir::toNativeSeparators(dbPath));
    if (!db.open()) {
        qCritical() << "LoggerWorker::create: Could not open database";
        return false;
    }
    QSqlQuery query(db);
    if (!query.exec("create table log (id integer primary key, date varchar, steps integer)")) {
        qCritical() << "LoggerWorker::create: Failed to create log table:" << query.lastError().text();
        return false;
    }
    if (!query.exec("create table tags (name varchar, value varchar, logid integer, foreign key(logid) references log(id))")) {
        qCritical() << "LoggerWorker::create: Failed to create tags table:" << query.lastError().text();
        return false;
    }

    Preferences::instance()->setLogDate(QDate::currentDate());
    QVariantMap tags;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nowUtc(now);
    nowUtc.setTimeSpec(Qt::UTC);
    tags.insert("secondsFromUtc", now.secsTo(nowUtc));
    return insertLog(steps, tags);
}

QString LoggerWorker::getArchiveName() {
    QString dir = QFileInfo(Platform::instance()->dbPath()).absolutePath();
    QString base = QDateTime::currentDateTime().toString("yyMMddhhmmss");
    QString archiveName = dir + "/" + base + ".adb";
    if (QFileInfo(archiveName).exists()) {
        qCritical() << "LoggerWorker::getArchiveName: Archive" << archiveName << "exists already";
        return QString();
    }
    return archiveName;
}
