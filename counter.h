#ifndef COUNTER_H
#define COUNTER_H

#include <sys/time.h>

#include <QObject>
#include <QAccelerometer>
#include <QTimer>
#include <QDebug>
#include <qmsystem2/qmdisplaystate.h>

#include "ring.h"

QTM_USE_NAMESPACE

#define COUNTER_NUM_PLAYERS 2

/// Step counter.
class Counter: public QObject {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY step)
    Q_PROPERTY(int rawCount READ rawCount WRITE setRawCount NOTIFY rawCountChanged)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(qreal calibration READ calibration WRITE setCalibration NOTIFY calibrationChanged)

public:
    explicit Counter(QObject *parent = 0);
    ~Counter();
    int count() {return stepCount;}
    bool running();
    qreal calibration() {return calibration_;}
    int rawCount() {return rawStepCount;}

signals:
    void step(int count);
    void runningChanged();
    void calibrationChanged(qreal value);
    void rawCountChanged(int value);

public slots:
    void measure();
    void pauseBlanking();
    Q_INVOKABLE void reset();
    void setRunning(bool running);
    void applicationActivated(bool active);
    void setCalibration(qreal value);
    void setRawCount(int value);

protected:
    QTimer *timer;
    QTimer *blankingTimer;
    QAccelerometer *accelerometer;
    MeeGo::QmDisplayState *displayState;
    Ring *ring;
    int peakCount;
    int rawStepCount; ///< Raw step count.
    int stepCount; ///< Calibrated step count.
    struct timeval lastPeakTime;
    long lastPeakTimeDiff;
    qreal calibration_; ///< Calibration coefficient.
};

#endif // COUNTER_H
