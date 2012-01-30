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

    /// Open URL with the default web browser.
    Q_INVOKABLE void openUrl(const QString &url);

    /// Load a text from a resource.
    /// Tries to resolve the resource name using the locale name as follows:
    /// - ":/texts/" + <language>_<Country> + "/" + key
    /// - ":/texts/" + <language> + "/" + key
    /// - ":/texts/" + key
    Q_INVOKABLE QString text(const QString &key);

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
