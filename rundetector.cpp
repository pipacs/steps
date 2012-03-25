#include <QtGlobal>
#include <QDateTime>
#include <QDebug>

#include "rundetector.h"

const qreal MIN_READING_DIFF = 50; ///< Minimum acceleration difference.
const qint64 MIN_WALKING_STEP_TIME_DIFF = 100; ///< Minimum time difference between steps while walking (ms).
const qint64 MIN_RUNNING_STEP_TIME_DIFF = 199; ///< Minimum time difference between steps while running (ms).
const int DATA_RATE = 20; ///< Accelerometer data rate (Hz).
const qreal RUNNING_READING_LIMIT = 300; ///< Accelerations larger than this are usually caused by running.

RunDetector::RunDetector(QObject *parent):
    Detector(parent),
    sensitivity_(100),
    increasing_(false),
    lastReading_(100000.),
    lastStepTime_(0),
    minStepTimeDiff_(MIN_WALKING_STEP_TIME_DIFF),
    stepCount_(0),
    totalReading_(0) {
    accelerometer_ = new QAccelerometer(this);
    accelerometer_->setProperty("alwaysOn", true);
    accelerometer_->addFilter(this);
    connect(accelerometer_, SIGNAL(activeChanged()), this, SIGNAL(runningChanged()));
}

RunDetector::~RunDetector() {
    accelerometer_->stop();
}

void RunDetector::setSensitivity(int sensitivity) {
    sensitivity_ = sensitivity;
    // minStepTimeDiff_ = MIN_STEP_TIME_DIFF - (sensitivity_ - 100);
    // qDebug() << "RunDetector::setSensitivity: minStepTimeDiff_ is" << minStepTimeDiff_;
    emit sensitivityChanged(sensitivity_);
}

int RunDetector::sensitivity() {
    return sensitivity_;
}

bool RunDetector::running() {
    return accelerometer_->isActive();
}

void RunDetector::setRunning(bool v) {
    if (v) {
        accelerometer_->setDataRate(DATA_RATE);
        accelerometer_->start();
    } else {
        accelerometer_->stop();
    }
}

bool RunDetector::filter(QAccelerometerReading *r) {
    qreal x = r->x();
    qreal y = r->y();
    qreal z = r->z();
    qreal reading = x * x + y * y + z * z;
    qreal readingDiff = reading - lastReading_;

    if (qAbs(readingDiff) < MIN_READING_DIFF) { // (MIN_READING_DIFF + (100 - sensitivity_) * 0.2)) {
        // qDebug() << "-" << qAbs(readingDiff);
        return true;
    }

    bool nowIncreasing = readingDiff > 0;
    if (increasing_ && !nowIncreasing) {
        qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
        qint64 timeDiff =  now - lastStepTime_;
        if (timeDiff > minStepTimeDiff_) {
            lastStepTime_ = now;
            emit step();
            adapt(reading);
       } else {
            qDebug() << "x" << timeDiff;
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
            minStepTimeDiff_ = MIN_RUNNING_STEP_TIME_DIFF;
            qDebug() << "RunDetector::adapt: Running";
        } else {
            minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
            qDebug() << "RunDetector::adapt: Walking";
        }
    }
}
