#ifndef PLATFORM_H
#define PLATFORM_H

#include <QObject>
#include <QString>
#include <QUrl>

#if defined(Q_OS_SYMBIAN)
#include <psmclientobserver.h>
#include <psmclient.h>
#endif

/// Platform abstractions.
class Platform: public QObject {
    Q_OBJECT

    /// Name of the operating system, like "harmattan"
    Q_PROPERTY(QString osName READ osName CONSTANT)

    /// Operating system version
    Q_PROPERTY(QString osVersion READ osVersion CONSTANT)

    /// Application version
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

    /// Universally unique device ID.
    Q_PROPERTY(QString deviceId READ deviceId CONSTANT)

    /// Power save mode.
    Q_PROPERTY(bool savePower READ savePower WRITE setSavePower NOTIFY savePowerChanged)

    /// Trace file name.
    Q_PROPERTY(QString traceFileName READ traceFileName CONSTANT)

    /// Build date.
    Q_PROPERTY(QString buildDate READ buildDate CONSTANT)

public:
    static Platform *instance();
    static void close();

    /// Operating systen name.
    QString osName();

    /// Operating system version
    QString osVersion();

    /// Application version.
    QString appVersion();

    /// Get full path for a sound file with base name "name".
    Q_INVOKABLE QUrl soundUrl(const QString &name);

    /// Full path name of the database file to use.
    Q_INVOKABLE QString dbPath();

    /// Is the disk containing the database full?
    bool dbFull();

    /// Device ID.
    QString deviceId();

    /// Get power save mode.
    bool savePower();

    /// Set power save mode.
    void setSavePower(bool v);

    /// Load a text from a resource.
    /// Tries to resolve the resource name using the locale name as follows:
    /// - ":/texts/" + <language>_<Country> + "/" + key
    /// - ":/texts/" + <language> + "/" + key
    /// - ":/texts/" + key
    Q_INVOKABLE QString text(const QString &key);

    /// Get trace file name.
    QString traceFileName() const;

    /// Enable/disable tracing to file.
    Q_INVOKABLE void traceToFile(bool enable);

    /// Delete trace file and disable tracing.
    Q_INVOKABLE void deleteTraceFile();

    /// Current time (seconds from Epoch).
    Q_INVOKABLE qint64 time();

    /// Build date.
    QString buildDate();

signals:
    void savePowerChanged();

protected:
    Platform();
    virtual ~Platform();

private:
#if defined(Q_OS_SYMBIAN)
    MPsmClientObserver *psmObserver;
    CPsmClient *psmClient;
#endif
};

#endif // PLATFORM_H
