#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QVariantHash>
#include <QSqlDatabase>

class LoggerWorker;

/// Log step counts and other events.
class Logger: public QObject {
    Q_OBJECT
public:
    explicit Logger(QObject *parent = 0);
    ~Logger();
    Q_INVOKABLE void log(int steps, const QVariantHash &tags);
    LoggerWorker *worker();
protected:
    LoggerWorker *worker_;

};

/// Do the real logging work.
class LoggerWorker: public QObject {
    Q_OBJECT
public:
    explicit LoggerWorker(QObject *parent = 0);
    ~LoggerWorker();
public slots:
    void doLog(int steps, const QVariantHash &tags);
protected:
    QSqlDatabase db;
};

#endif // LOGGER_H
