#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>

#include "logger.h"
#include "platform.h"

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

LoggerWorker::LoggerWorker(QObject *parent): QObject(parent) {
    QString absoluteDbPath = QFileInfo(Platform::instance()->dbPath()).absoluteFilePath();
    (void)QDir().mkpath(absoluteDbPath);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::toNativeSeparators(absoluteDbPath));
    if (!db.open()) {
        qCritical() << "LoggerWorker::LoggerWorker: Could not open" << Platform::instance()->dbPath() << ": Error" << db.lastError().text();
        return;
    }
    QSqlQuery query;
    if (!query.exec("create table if not exists log (id integer primary key, steps integer)")) {
        qCritical() << "LoggerWorker::LoggerWorker: Failed to create log table:" << query.lastError().text();
        return;
    }
    if (!query.exec("create table if not exists tags (name varchar, value varchar, logid integer, foreign key(logid) references log(id))")) {
        qCritical() << "LoggerWorker::LoggerWorker: Failed to create tags table:" << query.lastError().text();
    }
}

LoggerWorker::~LoggerWorker() {
    db.close();
}

void LoggerWorker::doLog(int steps, const QVariantHash &tags) {
    // FIXME: Implement me
}
