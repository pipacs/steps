#include <QtGlobal>
#include <QDateTime>
#include <QDebug>

#include "rundetector.h"

const qreal MIN_READING_DIFF = 50; ///< Minimum acceleration difference.
const qint64 MIN_WALKING_STEP_TIME_DIFF = 301; ///< Minimum time difference between steps while walking (ms).
const qint64 MIN_RUNNING_STEP_TIME_DIFF = 199; ///< Minimum time difference between steps while running (ms).
const int DATA_RATE_RUNNING = 20; ///< Accelerometer data rate for running (Hz).
const int DATA_RATE_WALKING = 10; ///< Accelerometer data rate for walking (Hz).
const qreal RUNNING_READING_LIMIT = 300; ///< Accelerations larger than this are usually caused by running.

RunDetector::RunDetector(QObject *parent): QObject(parent), running_(false) {
    reset();
    accelerometer_ = new QAccelerometer(this);
    accelerometer_->setProperty("alwaysOn", true);
    accelerometer_->addFilter(this);
}

RunDetector::~RunDetector() {
    accelerometer_->stop();
}

void RunDetector::setSensitivity(int sensitivity) {
    sensitivity_ = sensitivity;
    emit sensitivityChanged(sensitivity_);
}

int RunDetector::sensitivity() {
    return sensitivity_;
}

bool RunDetector::running() {
    return running_;
}

void RunDetector::setRunning(bool v) {
    if (v != running_) {
        if (v) {
            reset();
            accelerometer_->setDataRate(DATA_RATE_WALKING);
            accelerometer_->start();
        } else {
            accelerometer_->stop();
        }
    }
    running_ = v;
    emit runningChanged();
}

bool RunDetector::filter(QAccelerometerReading *r) {
    qreal x = r->x();
    qreal y = r->y();
    qreal z = r->z();
    qreal reading = x * x + y * y + z * z;
    qreal readingDiff = reading - lastReading_;

    // Filter out small changes
    if (qAbs(readingDiff) < MIN_READING_DIFF) {
        return true;
    }

    // Detect peak
    bool nowIncreasing = readingDiff > 0;
    if (increasing_ && !nowIncreasing) {
        qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
        qint64 timeDiff =  now - lastStepTime_;

        // If didn't peak too early, register a step, and adapt to current activity
        if (timeDiff > minStepTimeDiff_) {
            qDebug() << "+" << timeDiff;
            lastStepTime_ = now;
            emit step();
            adapt(reading);
        } else {
            qDebug() << "-" << timeDiff << "<" << minStepTimeDiff_;
        }
    }

    increasing_ = nowIncreasing;
    lastReading_ = reading;
    return true;
}

void RunDetector::adapt(qreal reading) {
    totalReading_ += reading;
    if ((++stepCount_ % 5) == 0) {
        qreal averageReading = totalReading_ / 5;
        totalReading_ = 0;
        if (averageReading > RUNNING_READING_LIMIT) {
            if (activity_ != Running) {
                activity_ = Running;
                minStepTimeDiff_ = MIN_RUNNING_STEP_TIME_DIFF;
                qDebug() << "RunDetector::adapt: Running, setting data rate to" << DATA_RATE_RUNNING;
                accelerometer_->stop();
                accelerometer_->setDataRate(DATA_RATE_RUNNING);
                accelerometer_->start();
            }
        } else {
            if (activity_ != Walking) {
                activity_ = Walking;
                minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
                qDebug() << "RunDetector::adapt: Walking, setting data rate to" << DATA_RATE_WALKING;
                accelerometer_->stop();
                accelerometer_->setDataRate(DATA_RATE_WALKING);
                accelerometer_->start();
            }
        }
    }
}


void RunDetector::reset() {
    sensitivity_ = 100;
    increasing_ = false;
    lastReading_ = 100000.;
    lastStepTime_ = 0;
    minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
    stepCount_ = 0;
    totalReading_ = 0;
    activity_ = Walking;
}
