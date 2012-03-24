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
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged)

public:
    explicit WalkDetector(QObject *parent = 0);
    ~WalkDetector();
    bool running();
    int sensitivity();

public slots:
    void measure();
    void reset();
    void setRunning(bool running);
    void setSensitivity(int value);

signals:
    void step();
    void runningChanged();
    void sensitivityChanged(int value);

public:
    QTimer *timer;
    QAccelerometer *accelerometer;
    Ring *ring;
    int peakCount;
    struct timeval lastPeakTime;
    int sensitivity_;
};


#endif // WALKDETECTOR_H
