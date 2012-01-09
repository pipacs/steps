#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QThread>
#include <QDateTime>
#include <QVariantMap>
#include <QtGlobal>

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
    ~LoggerWorker();

public slots:
    /// Log the current number of steps and some optional tags.
    void log(int steps, const QVariantMap &tags);

    /// Close and archive current database.
    void archive();

protected slots:
    /// Initialize database with schema.
    void onAddSchema();

protected:
    /// Archive database if it is older than one day.
    void archiveIfOld();

    /// Save log entry into an existing database: Insert new record or update the last one.
    /// On insert, lastInsertId is updated as well.
    void saveLog(int steps, const QVariantMap &tags);

    /// Insert a log entry into an existing database.
    /// @return Last insert ID.
    qlonglong insertLog(const QDateTime &now, int steps, const QVariantMap &tags);

    /// Update last log entry in an existing database.
    void updateLog(const QDateTime &now, int steps);

    /// Get the name of new archive file.
    QString getArchiveName();

    /// Return database, create it if doesn't exist.
    Database *db();

    QDateTime lastDate;
    int lastSteps;
    Database *db_;
    int logCount;
    bool diskFull;
    qlonglong lastInsertId;
};

#endif // LOGGER_H
