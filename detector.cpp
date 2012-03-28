#include <QtGlobal>
#include <QDateTime>
#include <QDebug>

#include "detector.h"

const qreal MIN_READING_DIFF = 50; ///< Minimum acceleration difference.
const qint64 MIN_WALKING_STEP_TIME_DIFF = 301; ///< Minimum time difference between steps while walking (ms).
const qint64 MIN_RUNNING_STEP_TIME_DIFF = 199; ///< Minimum time difference between steps while running (ms).
const int DATA_RATE_RUNNING = 20; ///< Accelerometer data rate for running (Hz).
const int DATA_RATE_WALKING = 10; ///< Accelerometer data rate for walking (Hz).
const qreal RUNNING_READING_LIMIT = 300; ///< Accelerations larger than this are usually caused by running.
const qint64 IDLE_TIME = 2000; ///< Set activity to Idle after this time (ms).
const qint64 IDLE_CHECK_INTERVAL = 1500; ///< Interval for checking for Idle (ms).

Detector::Detector(QObject *parent): QObject(parent), running_(false) {
    accelerometer_ = new QAccelerometer(this);
    accelerometer_->setProperty("alwaysOn", true);
    accelerometer_->addFilter(this);
    reset();
    idleTimer_ = new QTimer(this);
    idleTimer_->setSingleShot(false);
    idleTimer_->setInterval(IDLE_CHECK_INTERVAL);
    connect(idleTimer_, SIGNAL(timeout()), this, SLOT(checkIdle()));
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
            idleTimer_->start();
            accelerometer_->setDataRate(DATA_RATE_WALKING);
            accelerometer_->start();
        } else {
            accelerometer_->stop();
            idleTimer_->stop();
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

    // Filter out small changes
    if (qAbs(readingDiff) < MIN_READING_DIFF) {
        return true;
    }

    // Detect peak
    bool nowIncreasing = readingDiff > 0;
    if (increasing_ && !nowIncreasing) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 timeDiff =  now - lastStepTime_;

        // If didn't peak too early, register a step, and adapt to current activity
        if (timeDiff > minStepTimeDiff_) {
            qDebug() << "+" << timeDiff;
            lastStepTime_ = now;
            if (activity_ == Idle) {
                setActivity(Walking);
            }
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

void Detector::adapt(qreal reading) {
    totalReading_ += reading;
    if ((++stepCount_ % 5) == 0) {
        qreal averageReading = totalReading_ / 5;
        totalReading_ = 0;
        if (averageReading > RUNNING_READING_LIMIT) {
            if (activity_ != Running) {
                setActivity(Running);
                minStepTimeDiff_ = MIN_RUNNING_STEP_TIME_DIFF;
                qDebug() << "Detector::adapt: Running, setting data rate to" << DATA_RATE_RUNNING;
                accelerometer_->stop();
                accelerometer_->setDataRate(DATA_RATE_RUNNING);
                accelerometer_->start();
            }
        } else {
            if (activity_ != Walking) {
                setActivity(Walking);
                minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
                qDebug() << "Detector::adapt: Walking, setting data rate to" << DATA_RATE_WALKING;
                accelerometer_->stop();
                accelerometer_->setDataRate(DATA_RATE_WALKING);
                accelerometer_->start();
            }
        }
    }
}

void Detector::reset() {
    sensitivity_ = 100;
    increasing_ = false;
    lastReading_ = 100000.;
    lastStepTime_ = 0;
    minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
    stepCount_ = 0;
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

void Detector::checkIdle() {
    if (QDateTime::currentMSecsSinceEpoch() - lastStepTime_ > IDLE_TIME) {
        setActivity(Idle);
    }
}
