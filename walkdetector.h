#ifndef WALKDETECTOR_H
#define WALKDETECTOR_H

#include <sys/time.h>

#include <QObject>
#include <QAccelerometer>
#include <QTimer>
#include <QDebug>
#include <qplatformdefs.h>

#if defined(MEEGO_EDITION_HARMATTAN)
#include <qmsystem2/qmdisplaystate.h>
#endif

#include "detector.h"
#include "ring.h"

QTM_USE_NAMESPACE

/// Step detector for walking.
class WalkDetector: public Detector {
    Q_OBJECT

public:
    explicit WalkDetector(QObject *parent = 0);
    ~WalkDetector();
    bool running();
    int sensitivity();

signals:
    void step();
    void runningChanged();
    void sensitivityChanged(int value);

public slots:
    void measure();
    Q_INVOKABLE void reset();
    void setRunning(bool running);
    void setSensitivity(int value);

public:
    QTimer *timer;
    QAccelerometer *accelerometer;
    Ring *ring;
    int peakCount;
    struct timeval lastPeakTime;
    int sensitivity_;
};


#endif // WALKDETECTOR_H
