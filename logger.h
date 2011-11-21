#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QVariantHash>
#include <QSqlDatabase>
#include <QThread>
#include <QDateTime>

class LoggerWorker;

/// Log step counts and other events.
class Logger: public QObject {
    Q_OBJECT

public:
    static Logger *instance();
    static void close();
    Q_INVOKABLE void log(int steps, const QVariantHash &tags);

protected:
    explicit Logger(QObject *parent = 0);
    ~Logger();
    LoggerWorker *worker;
    QThread *workerThread;
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
    void create();
    QSqlDatabase db;
    QDateTime lastDate;
    int lastSteps;
    QVariantHash lastTags;
};

#endif // LOGGER_H
