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

    /// Log step count, with optional tags.
    Q_INVOKABLE void log(int steps, const QVariantMap &tags);

    /// Close and archive current log file, open new one.
    Q_INVOKABLE void archive();

    /// Upgrade old databases to current format
    Q_INVOKABLE void upgrade();

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
    /// Log step count, with optional tags.
    void log(int steps, const QVariantMap &tags);

    /// Close and archive current log file, open new one.
    void archive();

    /// Upgrade old databases to current format.
    void upgrade();

protected slots:
    /// Initialize database with schema.
    void onAddSchema();

protected:
    /// Archive database if it is older than one day.
    void archiveIfOld();

    /// Save log entry into an existing database: Insert new record or update the last one.
    void saveLog(int steps, const QVariantMap &tags);

    /// Insert a log entry into an existing database.
    /// If logging was successful, set lastInsertId to the last record ID and totalSteps to steps.
    void insertLog(const QDateTime &now, int steps, const QVariantMap &tags);

    /// Update last log entry in an existing database.
    /// If logging was successful, increase totalSteps by steps.
    void updateLog(const QDateTime &now, int steps);

    /// Get the name of new archive file.
    QString getArchiveName();

    /// Return database, create it if doesn't exist.
    Database *db();

    /// Upgrade "B" format database file to "C".
    void upgradeDbToDc(const QString &srcName);

    Database *database;     ///< Database, created on demand.
    int logCount;           ///< The number of times saveLog() has been called.
    bool diskFull;          ///< Was the disk full when we last checked.
    qlonglong lastInsertId; ///< Record ID of the last insert.
    int totalSteps;         ///< Total number of steps inserted in the last insert.
    QDateTime lastLogTime;  ///< Time of last logging.
};

#endif // LOGGER_H
