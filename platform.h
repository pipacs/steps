#ifndef PLATFORM_H
#define PLATFORM_H

#include <QObject>
#include <QString>
#include <QUrl>

/** Platform abstractions. */
class Platform: public QObject {
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)

public:
    static Platform *instance();
    static void close();
    QString version();
    Q_INVOKABLE QUrl soundUrl(const QString &name);
    Q_INVOKABLE void browse(const QString &url);

protected:
    Platform();
    ~Platform();
};

#endif // PLATFORM_H
