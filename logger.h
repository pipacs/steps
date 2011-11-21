#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QThread>
#include <QDateTime>
#include <QVariantMap>

class LoggerWorker;

/// Log step counts and other events.
class Logger: public QObject {
    Q_OBJECT

public:
    static Logger *instance();
    static void close();
    Q_INVOKABLE void log(int steps, const QVariantMap &tags);

protected:
    explicit Logger(QObject *parent = 0);
    virtual ~Logger();
    LoggerWorker *worker;
    QThread *workerThread;
};

/// Do the real logging work.
class LoggerWorker: public QObject {
    Q_OBJECT

public:
    explicit LoggerWorker(QObject *parent = 0);
    virtual ~LoggerWorker();

public slots:
    void doLog(int steps, const QVariantMap &tags);

protected:
    void create();
    QSqlDatabase db;
    QDateTime lastDate;
    int lastSteps;
};

#endif // LOGGER_H
