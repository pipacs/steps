#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QObject>
#include <QVariant>

/**
 * Shallow wrapper for QSettings.
 * Emits valueChanged signals when a setting value has changed, provides some settings as QML properties.
 */
class Preferences: public QObject {
    Q_OBJECT
    Q_PROPERTY(int rawCount READ rawCount WRITE setRawCount NOTIFY valueChanged)
    Q_PROPERTY(qreal calibration READ calibration WRITE setCalibration NOTIFY valueChanged)

public:
    static Preferences *instance();
    static void close();
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setValue(const QString &key, const QVariant &value);
    void apply();

    int rawCount() {return value("rawcount", 0).toInt();}
    void setRawCount(int v) {setValue("rawcount", v);}
    qreal calibration() {return value("calibration", 1.0).toReal();}
    void setCalibration(qreal v) {setValue("calibration", v);}

signals:
    void valueChanged(const QString &key);

protected:
    Preferences();
};

#endif // PREFERENCES_H
