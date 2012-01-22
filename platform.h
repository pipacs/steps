#ifndef PLATFORM_H
#define PLATFORM_H

#include <QObject>
#include <QString>
#include <QUrl>

/// Platform abstractions.
class Platform: public QObject {
    Q_OBJECT

    /// Name of the operating system, like "harmattan"
    Q_PROPERTY(QString osName READ osName CONSTANT)

    /// Application version
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

    /// Universally unique device ID.
    Q_PROPERTY(QString deviceId READ deviceId CONSTANT)

    /// Power save mode.
    Q_PROPERTY(bool savePower READ savePower WRITE setSavePower NOTIFY savePowerChanged)

public:
    static Platform *instance();
    static void close();

    /// Platform name, like "symbian" or "harmattan".
    QString osName();

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

signals:
    void savePowerChanged();

protected:
    Platform();
};

#endif // PLATFORM_H
