#include <math.h>
#include <sys/time.h>
#include <assert.h>
#include <QDebug>
#include <QDateTime>
#include "counter.h"

const int Span = 2000; // Time span for detecting peaks, milliseconds
const int Interval = 100; // Accelerometer reading interval, milliseconds
const int BlankingInterval = 5000; // Display blanking timer interval, milliseconds
const qreal MinimumRise = 50.0f; // Minimum diff in accelerometer readings (any direction)
const long DefaultPeakTimeDiff = 700; // Default time difference between peaks, milliseconds
const long MinimumPeakTimeDiff = 150; // Minimum time difference between peaks, milliseconds

// Get time difference in milliseconds
long timeDiff(const struct timeval &start, const struct timeval &end);

Counter::Counter(QObject *parent): QObject(parent), calibration_(1.0), sensitivity_(100) {
#if defined(MEEGO_EDITION_HARMATTAN)
    displayState = 0;
#endif
    ring = new Ring(Span / Interval, MinimumRise);
    reset();

    accelerometer = new QAccelerometer(this);
    if (!accelerometer->isActive()) {
        accelerometer->start();
    }
    if (!accelerometer->isActive()) {
        qCritical() << "Accelerometer sensor didn't start!" << endl;
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
    qDebug() << "Counter::~Counter";
    accelerometer->stop();
    // delete accelerometer;
#if defined(MEEGO_EDITION_HARMATTAN)
    delete displayState;
#endif
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

    if (peakTimeDiff < MinimumPeakTimeDiff) {
        // Peaked too early
        return;
    }

    lastPeakTime.tv_sec = peakTime.tv_sec;
    lastPeakTime.tv_usec = peakTime.tv_usec;
    ++peakCount;

    if (peakCount % 2) {
        // Every second peak is a (raw) step
        ++rawStepCount;
        emit rawCountChanged(rawStepCount);
        int newStepCount = (int)(rawStepCount * calibration_);
        if (newStepCount != stepCount) {
            stepCount = newStepCount;
            emit step(newStepCount);
        }
    }
}

void Counter::reset() {
    stepCount = peakCount = rawStepCount = 0;
    gettimeofday(&lastPeakTime, 0);
    ring->clear();
    emit rawCountChanged(0);
    emit step(0);
}

void Counter::pauseBlanking() {
#if defined(MEEGO_EDITION_HARMATTAN)
    if (!displayState) {
        displayState = new MeeGo::QmDisplayState();
    }
    (void)displayState->setBlankingPause();
#endif
}

bool Counter::running() {
    return timer->isActive();
}

void Counter::setRunning(bool run) {
    if (run == running()) {
        return;
    }
    if (run) {
        accelerometer->start();
        timer->start();
    } else {
        timer->stop();
        accelerometer->stop();
    }
    emit runningChanged();
}

void Counter::applicationActivated(bool active) {
    if (!active) {
        setRunning(false);
    }
}

void Counter::setCalibration(qreal c) {
    calibration_ = c;
    int newStepCount = (int)(rawStepCount * calibration_);
    if (newStepCount != stepCount) {
        stepCount = newStepCount;
        emit step(newStepCount);
    }
    emit calibrationChanged(c);
}

void Counter::setRawCount(int value) {
    rawStepCount = value;
    int newStepCount = (int)(rawStepCount * calibration_);
    if (newStepCount != stepCount) {
        stepCount = newStepCount;
        emit step(newStepCount);
    }
    emit rawCountChanged(rawStepCount);
}

void Counter::setSensitivity(int value) {
    qDebug() << "Counter::setSensitivity" << value;
    sensitivity_ = value;
    ring->setMinimumRise(MinimumRise + 100 - value);
    emit sensitivityChanged(value);
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

