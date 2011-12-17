#ifndef PLATFORM_H
#define PLATFORM_H

#include <QObject>
#include <QString>
#include <QUrl>

/// Platform abstractions.
class Platform: public QObject {
    Q_OBJECT
    Q_PROPERTY(QString osName READ osName CONSTANT)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

public:
    static Platform *instance();
    static void close();

    /// Platform name, like "symbian" or "harmattan".
    QString osName();

    /// Application version.
    QString appVersion();

    /// Get full path for a sound file with base name "name".
    Q_INVOKABLE QUrl soundUrl(const QString &name);

    /// Full path name of the database directory to use.
    Q_INVOKABLE QString dbPath();

protected:
    Platform();
};

#endif // PLATFORM_H
