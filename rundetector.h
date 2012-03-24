#ifndef RUNDETECTOR_H
#define RUNDETECTOR_H

#include <QAccelerometer>

#include "detector.h"

QTM_USE_NAMESPACE

/// Step detector for running.
class RunDetector: public Detector, public QAccelerometerFilter {
    Q_OBJECT

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
    qreal lastReading_; /// Last absolute accelerometer reading.
    qint64 lastPeakTime_; /// Time of the last peak (ms since Epoch).

private slots:
    bool filter(QAccelerometerReading* r);
};

#endif // RUNDETECTOR_H
