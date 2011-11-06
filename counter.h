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

class Counter: public QObject {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY step)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit Counter(QObject *parent = 0);
    ~Counter();
    int count() {return stepCount;}
    bool running();

signals:
    void step(unsigned count);
    void runningChanged();

public slots:
    void measure();
    void pauseBlanking();
    Q_INVOKABLE void reset();
    void setRunning(bool running);
    void applicationActivated(bool active);

protected:
    QTimer *timer;
    QTimer *blankingTimer;
    QAccelerometer *accelerometer;
    MeeGo::QmDisplayState *displayState;
    Ring *ring;
    int peakCount;
    int stepCount;
    struct timeval lastPeakTime;
    long lastPeakTimeDiff;
};

#endif // COUNTER_H
