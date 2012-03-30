#include <QtGlobal>
#include <QDebug>

#include "detector.h"

const qreal MIN_READING_DIFF = 50; ///< Minimum acceleration difference.
const qint64 MIN_WALKING_STEP_TIME_DIFF = 301; ///< Minimum time difference between steps while walking (ms).
const qint64 MIN_RUNNING_STEP_TIME_DIFF = 150; ///< Minimum time difference between steps while running (ms).
const int DATA_RATE_RUNNING = 20; ///< Accelerometer data rate for running (Hz).
const int DATA_RATE_WALKING = 10; ///< Accelerometer data rate for walking (Hz).
const qreal RUNNING_READING_LIMIT = 300; ///< Accelerations larger than this are usually caused by running.
const qint64 IDLE_TIME = 2000; ///< Set activity to Idle after this time (ms).

Detector::Detector(QObject *parent): QObject(parent), running_(false), runningStepTimeDiff_(MIN_RUNNING_STEP_TIME_DIFF) {
    accelerometer_ = new QAccelerometer(this);
    accelerometer_->setProperty("alwaysOn", true);
    accelerometer_->addFilter(this);
    reset();
}

Detector::~Detector() {
    accelerometer_->stop();
}

void Detector::setSensitivity(int sensitivity) {
    sensitivity_ = sensitivity;
    emit sensitivityChanged(sensitivity_);
}

int Detector::sensitivity() {
    return sensitivity_;
}

bool Detector::running() {
    return running_;
}

void Detector::setRunning(bool v) {
    if (v != running_) {
        if (v) {
            reset();
            accelerometer_->setDataRate(DATA_RATE_WALKING);
            accelerometer_->start();
        } else {
            accelerometer_->stop();
            setActivity(Idle);
        }
        running_ = v;
        emit runningChanged();
    }
}

bool Detector::filter(QAccelerometerReading *r) {
    qreal x = r->x();
    qreal y = r->y();
    qreal z = r->z();
    qreal reading = x * x + y * y + z * z;
    qreal readingDiff = reading - lastReading_;

    // Get the time of the reading, work around system clock going backwards
    qint64 now = (qint64)r->timestamp() / 1000;
    if (now < lastStepTime_) {
        lastStepTime_ = now;
    }

    // Maintain average, guess current activity
    adapt(reading, now);

    // Filter out small changes
    if (qAbs(readingDiff) < MIN_READING_DIFF) {
        return true;
    }

    // Detect peak
    bool nowIncreasing = readingDiff > 0;
    if (increasing_ && !nowIncreasing) {
        // If didn't peak too early, register a step, and adapt to current activity
        qint64 timeDiff = now - lastStepTime_;
        if (timeDiff > minStepTimeDiff_) {
            qDebug() << "+" << timeDiff;
            lastStepTime_ = now;
            if (activity_ == Idle) {
                setActivity(Walking);
            }
            emit step();
        } else {
            qDebug() << "-" << timeDiff << "<" << minStepTimeDiff_;
        }
    }

    increasing_ = nowIncreasing;
    lastReading_ = reading;
    return true;
}

void Detector::adapt(qreal reading, qint64 timeStamp) {
    totalReading_ += reading;
    if (++readingCount_ % 20) {
        return;
    }

    qreal averageReading = totalReading_ / 20;
    totalReading_ = 0;

    bool isStepping = (timeStamp - lastStepTime_) < IDLE_TIME;
    bool isRunning = false;

    if (averageReading > RUNNING_READING_LIMIT) {
        isRunning = true;
        if (activity_ != Running) {
            minStepTimeDiff_ = runningStepTimeDiff_;
            qDebug() << "Detector::adapt: Running, setting data rate to" << DATA_RATE_RUNNING;
            accelerometer_->stop();
            accelerometer_->setDataRate(DATA_RATE_RUNNING);
            accelerometer_->start();
        }
    } else {
        if (activity_ == Running) {
            minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
            qDebug() << "Detector::adapt: Walking or Idle, setting data rate to" << DATA_RATE_WALKING;
            accelerometer_->stop();
            accelerometer_->setDataRate(DATA_RATE_WALKING);
            accelerometer_->start();
        }
    }

    setActivity(isStepping? (isRunning? Running: Walking): Idle);
}

void Detector::reset() {
    sensitivity_ = 100;
    increasing_ = false;
    lastReading_ = 100000.;
    lastStepTime_ = 0;
    minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
    readingCount_ = 0;
    totalReading_ = 0;
    setActivity(Idle);
}

Detector::Activity Detector::activity() {
    return activity_;
}

void Detector::setActivity(Activity v) {
    if (activity_ != v) {
        activity_ = v;
        emit activityChanged();
    }
}

int Detector::runningStepTimeDiff() {
    return runningStepTimeDiff_;
}

void Detector::setRunningStepTimeDiff(int diff) {
    qDebug() << "Detector::setRunningStepTimeDiff" << diff;
    if (diff != runningStepTimeDiff_) {
        runningStepTimeDiff_ = diff;
        emit runningStepTimeDiffChanged();
    }
}
