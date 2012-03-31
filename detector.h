#ifndef RUNDETECTOR_H
#define RUNDETECTOR_H

#include <QAccelerometer>
#include <QTimer>
#include <QThread>

QTM_USE_NAMESPACE

class DetectorWorker;

class Detector: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_ENUMS(Activity)
    Q_PROPERTY(int activity READ activity NOTIFY activityChanged)
    Q_PROPERTY(int runningStepTimeDiff READ runningStepTimeDiff WRITE setRunningStepTimeDiff NOTIFY runningStepTimeDiffChanged)
    Q_PROPERTY(int minReadingDiff READ minReadingDiff WRITE setMinReadingDiff NOTIFY minReadingDiffChanged)

public:
    enum Activity {
        Idle,
        Unknown,
        Walking,
        Running
    };

    explicit Detector(QObject *parent = 0);
    ~Detector();
    bool running() {return running_;}
    int activity() {return activity_;}
    int runningStepTimeDiff() {return runningStepTimeDiff_;}
    int minReadingDiff() {return minReadingDiff_;}

signals:
    void step();
    void runningChanged(bool value);
    void activityChanged(int value);
    void runningStepTimeDiffChanged(int value);
    void minReadingDiffChanged(int value);

public slots:
    void setRunning(bool value);
    void setRunningStepTimeDiff(int value);
    void setMinReadingDiff(int value);

    void onRunningChanged(bool value);
    void onActivityChanged(int activity);
    void onRunningStepTimeDiffChanged(int diff);
    void onMinReadingDiffChanged(int diff);

public:
    int activity_; ///< Current activity.
    bool running_; ///< True if the detector is running.
    int runningStepTimeDiff_; ///< Minimum time difference between steps, while running (ms).
    int minReadingDiff_; ///< Minimum reading delta to consider a step.
    DetectorWorker *worker_; ///< The real detector.
    QThread *workerThread_; ///< Detector thread.
};

/// Step detector worker.
class DetectorWorker: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int activity READ activity NOTIFY activityChanged)
    Q_PROPERTY(int runningStepTimeDiff READ runningStepTimeDiff WRITE setRunningStepTimeDiff NOTIFY runningStepTimeDiffChanged)
    Q_PROPERTY(int minReadingDiff READ minReadingDiff WRITE setMinReadingDiff NOTIFY minReadingDiffChanged)

public:
    explicit DetectorWorker(QObject *parent = 0);
    void init(); ///< Initialize the accelerometer.

    bool running() {return running_;}
    int activity() {return (int)activity_;}
    int runningStepTimeDiff() {return runningStepTimeDiff_;}
    int minReadingDiff() {return minReadingDiff_;}

signals:
    void step();
    void runningChanged(bool value);
    void activityChanged(int value);
    void runningStepTimeDiffChanged(int value);
    void minReadingDiffChanged(int value);

public slots:
    void setRunning(bool value);
    void setActivity(Detector::Activity value);
    void setRunningStepTimeDiff(int value);
    void setMinReadingDiff(int value);
    void detect(); ///< Detect step.
    void reset(); ///< Reset all parameters to default.

    /// Guess current activity (running or walking) and adapt parameters to it.
    void adapt(qreal reading, qint64 timeStamp);

public:
    QAccelerometer *accelerometer_;
    bool increasing_; ///< True if accelerometer readings are increasing.
    qreal lastReading_; ///< Last accelerometer reading.
    qint64 lastStepTime_; ///< Time of the last step (ms since Epoch).
    qint64 minStepTimeDiff_; ///< Minimum time between steps (ms).
    unsigned readingCount_; ///< Reading count.
    qreal totalReading_; ///< Sum of the last N accelerometer readings.
    Detector::Activity activity_; ///< Current activity.
    bool running_; ///< True if the detector is running.
    int runningStepTimeDiff_; ///< Minimum time difference between steps, while running (ms).
    int minReadingDiff_; ///< Minimum reading delta to consider a step.
    int stepCount_; ///< Total step count.
};

#endif // RUNDETECTOR_H
