#ifndef RUNDETECTOR_H
#define RUNDETECTOR_H

#include <QAccelerometer>
#include <QTimer>

QTM_USE_NAMESPACE

/// Step detector.
class Detector: public QObject, public QAccelerometerFilter {
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged)
    Q_ENUMS(Activity)
    Q_PROPERTY(Activity activity READ activity NOTIFY activityChanged)
    Q_PROPERTY(int runningStepTimeDiff READ runningStepTimeDiff WRITE setRunningStepTimeDiff NOTIFY runningStepTimeDiffChanged)

public:
    enum Activity {
        Idle,
        Unknown,
        Walking,
        Running
    };

    explicit Detector(QObject *parent = 0);
    ~Detector();
    bool running();
    int sensitivity();
    Activity activity();
    int runningStepTimeDiff();

signals:
    void step();
    void runningChanged();
    void sensitivityChanged(int value);
    void activityChanged();
    void runningStepTimeDiffChanged();

public slots:
    void setRunning(bool running);
    void setSensitivity(int sensitivity);
    void setActivity(Activity activity);
    void setRunningStepTimeDiff(int diff);

    /// Guess current activity (running or walking) and adapt parameters to it.
    void adapt(qreal reading, qint64 timeStamp);

    /// Reset all parameters to default.
    void reset();

public:
    int sensitivity_;
    QAccelerometer *accelerometer_;
    bool increasing_; ///< True if accelerometer readings are increasing.
    qreal lastReading_; ///< Last accelerometer reading.
    qint64 lastStepTime_; ///< Time of the last step (ms since Epoch).
    qint64 minStepTimeDiff_; ///< Minimum time between steps (ms).
    unsigned readingCount_; ///< Reading count.
    qreal totalReading_; ///< Sum of the last N accelerometer readings.
    Activity activity_; ///< Current activity.
    bool running_; ///< True if the detector is running.
    int runningStepTimeDiff_; ///< Minimum time difference between steps, while running (ms).

private slots:
    bool filter(QAccelerometerReading *r);
};

#endif // RUNDETECTOR_H
