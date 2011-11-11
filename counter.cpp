#include <math.h>
#include <sys/time.h>
#include <assert.h>
#include <QDebug>
#include <QDateTime>
#include "counter.h"

const int Span = 2000; // Time span for detecting peaks, milliseconds
const int Interval = 100; // Accelerometer reading interval, milliseconds
const int BlankingInterval = 5000; // Display blanking timer interval, milliseconds
const qreal MinRise = 30.0f; // Minimum diff in accelerometer readings (any direction)
const long DefaultPeakTimeDiff = 700; // Default time difference between peaks, milliseconds

// Get time difference in milliseconds
long timeDiff(const struct timeval &start, const struct timeval &end);

Counter::Counter(QObject *parent): QObject(parent), displayState(0), calibration_(1.0) {
    ring = new Ring(Span / Interval, MinRise);
    reset();

    accelerometer = new QAccelerometer(this);
    if (!accelerometer->isActive()) {
        accelerometer->start();
    }
    if (!accelerometer->isActive()) {
        qCritical() << "accelerometer sensor didn't start!" << endl;
        return;
    }

    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(Interval);
    connect(timer, SIGNAL(timeout()), this, SLOT(measure()));

    blankingTimer = new QTimer(this);
    blankingTimer->setSingleShot(false);
    blankingTimer->setInterval(BlankingInterval);
    connect(blankingTimer, SIGNAL(timeout()), this, SLOT(pauseBlanking()));
    blankingTimer->start();
}

Counter::~Counter() {
    delete displayState;
    delete ring;
}

void Counter::measure() {
    QAccelerometerReading *reading = accelerometer->reading();
    qreal x;
    qreal y;
    qreal z;
    struct timeval peakTime;
    long peakTimeDiff;

    if (!reading) {
        return;
    }

    x = reading->x();
    y = reading->y();
    z = reading->z();
    qreal measurement = x * x + y * y + z * z;
    ring->add(measurement);

    if (-1 == ring->findPeak()) {
        return;
    }

    ring->clear();
    gettimeofday(&peakTime, 0);
    peakTimeDiff = timeDiff(lastPeakTime, peakTime);

    if (peakTimeDiff < (lastPeakTimeDiff / 2)) {
        qDebug() << "  Peaked too early: after" << peakTimeDiff << "ms";
    }

    lastPeakTime.tv_sec = peakTime.tv_sec;
    lastPeakTime.tv_usec = peakTime.tv_usec;
    lastPeakTimeDiff = (peakTimeDiff > DefaultPeakTimeDiff)? DefaultPeakTimeDiff: peakTimeDiff;
    ++peakCount;

    if (peakCount % 2) {
        ++rawStepCount;
        qDebug() << rawStepCount << ": Step after" << peakTimeDiff << "ms";
        emit rawCountChanged(rawStepCount);
        int newStepCount = (int)(rawStepCount * calibration_);
        if (newStepCount != stepCount) {
            stepCount = newStepCount;
            emit step(newStepCount);
        }
        // QDateTime now = QDateTime::currentDateTimeUtc();
        // logger.log(QString("%1,%2,%3,%4,%5").arg(now.toString("yyyy-MM-ddThh:mm:ss.zzz")).arg(x).arg(y).arg(z).arg(reading));
    }
}

void Counter::reset() {
    stepCount = peakCount = 0;
    lastPeakTimeDiff = DefaultPeakTimeDiff;
    lastPeakTime.tv_sec = lastPeakTime.tv_usec = 0;
    ring->clear();
    emit step(0);
}

void Counter::pauseBlanking() {
    if (!displayState) {
        displayState = new MeeGo::QmDisplayState;
    }
    (void)displayState->setBlankingPause();
}

bool Counter::running() {
    return timer->isActive();
}

void Counter::setRunning(bool run) {
    if (run == running()) {
        return;
    }
    if (run) {
        lastPeakTimeDiff = DefaultPeakTimeDiff;
        timer->start();
    } else {
        timer->stop();
    }
    emit runningChanged();
}

void Counter::applicationActivated(bool active) {
    if (!active) {
        setRunning(false);
    }
}

long timeDiff(const struct timeval &start_time, const struct timeval &end_time) {
    struct timeval difference;
    difference.tv_sec = end_time.tv_sec - start_time.tv_sec ;
    difference.tv_usec = end_time.tv_usec - start_time.tv_usec;
    while (difference.tv_usec < 0) {
        difference.tv_usec += 1000000;
        difference.tv_sec -= 1;
    }
    return (long)((1000000LL * difference.tv_sec + difference.tv_usec) / 1000LL);
}

