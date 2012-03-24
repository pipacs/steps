#include <math.h>
#include <sys/time.h>
#include <assert.h>
#include <QDebug>
#include <QDateTime>

#include "walkdetector.h"

const int Span = 2000; // Time span for detecting peaks, milliseconds
const int Interval = 100; // Accelerometer reading interval, milliseconds
const qreal MinimumRise = 50.0f; // Minimum diff in accelerometer readings (any direction)
const long DefaultPeakTimeDiff = 700; // Default time difference between peaks, milliseconds
const long MinimumPeakTimeDiff = 150; // Minimum time difference between peaks, milliseconds

// Get time difference in milliseconds
long timeDiff(const struct timeval &start, const struct timeval &end);

WalkDetector::WalkDetector(QObject *parent): Detector(parent), sensitivity_(100) {
    ring = new Ring(Span / Interval, MinimumRise);
    reset();

    accelerometer = new QAccelerometer(this);
    accelerometer->setProperty("alwaysOn", true);
    if (!accelerometer->isActive()) {
        accelerometer->start();
    }
    if (!accelerometer->isActive()) {
        qCritical() << "Accelerometer sensor didn't start" << endl;
        return;
    }

    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(Interval);
    connect(timer, SIGNAL(timeout()), this, SLOT(measure()));
}

WalkDetector::~WalkDetector() {
    accelerometer->stop();
    delete ring;
}

void WalkDetector::measure() {
    QAccelerometerReading *reading = accelerometer->reading();
    struct timeval peakTime;
    long peakTimeDiff;

    if (!reading) {
        return;
    }

    qreal x = reading->x();
    qreal y = reading->y();
    qreal z = reading->z();
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

    // Every second peak is a step
    if (peakCount % 2) {
        emit step();
    }
}

void WalkDetector::reset() {
    peakCount = 0;
    gettimeofday(&lastPeakTime, 0);
    ring->clear();
}

bool WalkDetector::running() {
    return timer->isActive();
}

void WalkDetector::setRunning(bool run) {
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

void WalkDetector::setSensitivity(int value) {
    sensitivity_ = value;
    ring->setMinimumRise(MinimumRise + 100 - value);
    emit sensitivityChanged(value);
}

int WalkDetector::sensitivity() {
    return sensitivity_;
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

