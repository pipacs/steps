#ifndef RUNDETECTOR_H
#define RUNDETECTOR_H

#include <QAccelerometer>

#include "detector.h"

QTM_USE_NAMESPACE

/// Step detector.
class RunDetector: public Detector, public QAccelerometerFilter {
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged)

public:
    explicit RunDetector(QObject *parent = 0);
    ~RunDetector();
    bool running();
    int sensitivity();

signals:
    void step();
    void runningChanged();
    void sensitivityChanged(int value);

public slots:
    void setRunning(bool running);
    void setSensitivity(int sensitivity);

public:
    int sensitivity_;
    QAccelerometer *accelerometer_;
    bool increasing_; ///< True if absolute accelerometer readings are increasing.
    qreal lastReading_; ///< Last absolute accelerometer reading.
    qint64 lastStepTime_; ///< Time of the last step (ms since Epoch).
    qint64 minStepTimeDiff_; ///< Minimum time between steps (ms).

private slots:
    bool filter(QAccelerometerReading* r);
};

#endif // RUNDETECTOR_H
