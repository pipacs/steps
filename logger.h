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
    /// Log the current number of steps and some optional tags.
    void log(int steps, const QVariantMap &tags);

    /// Archive current database, create a new one.
    /// @param      steps   Step count to record in the newly created database.
    /// @return     True (success) or false (failure).
    bool archive(int steps);

protected:
    /// Check database, create it if doesn't exist, archive it if needed.
    /// @param  steps   Step count to recod in the newly created database.
    /// @return True if database exists or has been created/archived successfully, false otherwise.
    bool checkDb(int steps);

    /// Insert a log entry into an existing database.
    /// @return True if insertion succeeded, false otherwise.
    bool insertLog(int steps, const QVariantMap &tags);

    /// Create database.
    /// @param  steps   Step count to record in the newly created database.
    /// @return True (success) or false (failure).
    bool create(int steps);

    /// Find and return the name of the archive file.
    QString getArchiveName();

    QSqlDatabase db;
    QDateTime lastDate;
    int lastSteps;
};

#endif // LOGGER_H
