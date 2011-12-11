#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QThread>
#include <QDateTime>
#include <QVariantMap>

class LoggerWorker;
class Database;

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

public slots:
    /// Log the current number of steps and some optional tags.
    void log(int steps, const QVariantMap &tags);

    /// Close and archive current database.
    void archive();

protected slots:
    /// Initialize database with schema.
    void onInitialize();

protected:
    /// Archive database if it is older than one day.
    void archiveIfOld();

    /// Insert a log entry into an existing database.
    /// @return True if insertion succeeded, false otherwise.
    bool insertLog(int steps, const QVariantMap &tags);

    /// Get the name of new archive file.
    QString getArchiveName();

    Database *db;
    QDateTime lastDate;
    int lastSteps;
};

#endif // LOGGER_H
