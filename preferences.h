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

    /// Current activity type
    Q_PROPERTY(int activity READ activity WRITE setActivity NOTIFY valueChanged)

    /// Show/hide exit icon on main toolbar
    Q_PROPERTY(bool showExit READ showExit WRITE setShowExit NOTIFY valueChanged)

    /// Power saving.
    Q_PROPERTY(bool savePower READ savePower WRITE setSavePower NOTIFY valueChanged)

    /// Trace to file.
    Q_PROPERTY(bool traceToFile READ traceToFile WRITE setTraceToFile NOTIFY valueChanged)

    /// Minimum time difference between steps, while running (ms).
    Q_PROPERTY(int runningStepTimeDiff READ runningStepTimeDiff WRITE setRunningStepTimeDiff NOTIFY valueChanged)

    /// Minimum difference between accelerometer readings to consider a step.
    Q_PROPERTY(int minReadingDiff READ minReadingDiff WRITE setMinReadingDiff NOTIFY valueChanged)

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

    int activity() {return value("activity").toInt();}
    void setActivity(int v) {setValue("activity", v);}

    bool showExit() {return value("showexit").toBool();}
    void setShowExit(bool v) {setValue("showexit", v);}

    bool savePower() {return value("savepower").toBool();}
    void setSavePower(bool v) {setValue("savepower", v);}

    bool traceToFile() {return value("tracetofile").toBool();}
    void setTraceToFile(bool v) {setValue("tracetofile", v);}

    int runningStepTimeDiff() {return value("runningsteptimediff").toInt();}
    void setRunningStepTimeDiff(int v) {setValue("runningsteptimediff", v);}

    int minReadingDiff() {return value("minreadingdiff").toInt();}
    void setMinReadingDiff(int v) {setValue("minreadingdiff", v);}

signals:
    void valueChanged(const QString &key);

protected:
    Preferences();
};

#endif // PREFERENCES_H
