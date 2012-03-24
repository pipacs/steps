#include <QtGlobal>
#include <QDateTime>
#include <QDebug>

#include "rundetector.h"

const qreal MIN_READING_DIFF = 50; ///< Minimum absolute acceleration difference.
const qint64 MIN_PEAK_TIME_DIFF = 100; ///< Minimum time difference between peeks (ms).

RunDetector::RunDetector(QObject *parent): Detector(parent), sensitivity_(100), increasing_(true), lastReading_(0.), lastPeakTime_(0) {
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

    if (qAbs(readingDiff) < MIN_READING_DIFF) {
        return true;
    }

    bool nowIncreasing = readingDiff > 0;
    if (increasing_ && !nowIncreasing) {
        qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
        if (now - lastPeakTime_ > MIN_PEAK_TIME_DIFF) {
            lastPeakTime_ = now;
            emit step();
        }
    }

    increasing_ = nowIncreasing;
    lastReading_ = reading;
    return true;
}
