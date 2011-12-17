#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QObject>
#include <QVariant>
#include <QDebug>
#include <QDate>

/// Shallow wrapper for QSettings.
/// Emits valueChanged signals when a setting value has changed, provides some settings as QML properties.
class Preferences: public QObject {
    Q_OBJECT
    Q_PROPERTY(int rawCount READ rawCount WRITE setRawCount NOTIFY valueChanged)
    Q_PROPERTY(qreal calibration READ calibration WRITE setCalibration NOTIFY valueChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY valueChanged)
    Q_PROPERTY(qreal sensitivity READ sensitivity WRITE setSensitivity NOTIFY valueChanged)

    /// Daily step count.
    Q_PROPERTY(int dailyCount READ dailyCount WRITE setDailyCount NOTIFY valueChanged)

    /// Current activity step count.
    Q_PROPERTY(int activityCount READ activityCount WRITE setActivityCount NOTIFY valueChanged)

    /// Date of the daily step count (arbitrary string).
    Q_PROPERTY(QString dailyCountDate READ dailyCountDate WRITE setDailyCountDate NOTIFY valueChanged)

    /// Date of last log. If it's not the current date, the log file should be archived.
    Q_PROPERTY(QDate logDate READ logDate WRITE setLogDate NOTIFY valueChanged)

public:
    static Preferences *instance();
    static void close();

    Q_INVOKABLE QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);
    void apply();

    int rawCount() {return value("rawcount", 0).toInt();}
    void setRawCount(int v) {setValue("rawcount", v);}

    qreal calibration() {return value("calibration", 1.0).toReal();}
    void setCalibration(qreal v) {setValue("calibration", v);}

    bool muted() {return value("muted", false).toBool();}
    void setMuted(bool v) {setValue("muted", v);}

    int sensitivity() {return value("sensitivity", 100).toInt();}
    void setSensitivity(int v) {setValue("sensitivity", v);}

    QDate logDate() {return value("logdate", QDate::currentDate()).toDate();}
    void setLogDate(const QDate &v) {setValue("logdate", v);}

    int dailyCount() {return value("dailycount", 0).toInt();}
    void setDailyCount(int v) {setValue("dailycount", v);}

    int activityCount() {return value("activitycount", 0).toInt();}
    void setActivityCount(int v) {setValue("activitycount", v);}

    QString dailyCountDate() {return value("dailycountdate").toString();}
    void setDailyCountDate(const QString &v) {setValue("dailycountdate", v);}

signals:
    void valueChanged(const QString &key);

protected:
    Preferences();
};

#endif // PREFERENCES_H
