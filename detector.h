#ifndef DETECTOR_H
#define DETECTOR_H

#include <sys/time.h>

#include <QObject>
#include <QAccelerometer>
#include <QTimer>
#include <QDebug>
#include <qplatformdefs.h>

#if defined(MEEGO_EDITION_HARMATTAN)
#include <qmsystem2/qmdisplaystate.h>
#endif

#include "ring.h"

QTM_USE_NAMESPACE

/// Step detector.
class Detector: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged)

public:
    explicit Detector(QObject *parent = 0);
    ~Detector();
    bool running();
    int sensitivity() {return sensitivity_;}

signals:
    void step();
    void runningChanged();
    void sensitivityChanged(int value);

public slots:
    void measure();
    Q_INVOKABLE void reset();
    void setRunning(bool running);
    void setSensitivity(int value);

protected:
    QTimer *timer;
    QAccelerometer *accelerometer;
    Ring *ring;
    int peakCount;
    struct timeval lastPeakTime;
    int sensitivity_; ///< Default: 100.
};

#endif // DETECTOR_H
