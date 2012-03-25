#include <QtGlobal>
#include <QDateTime>
#include <QDebug>

#include "rundetector.h"

const qreal MIN_READING_DIFF = 50; ///< Minimum absolute acceleration difference.
const qint64 DEFAULT_MIN_STEP_TIME_DIFF = 199; ///< Default minimum time difference between steps (ms).
const int DATA_RATE = 20; ///< Accelerometer data rate (Hz).

RunDetector::RunDetector(QObject *parent): Detector(parent), sensitivity_(100), increasing_(false), lastReading_(100000.), lastStepTime_(0), minStepTimeDiff_(DEFAULT_MIN_STEP_TIME_DIFF) {
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
    minStepTimeDiff_ = DEFAULT_MIN_STEP_TIME_DIFF - (sensitivity_ - 100);
    qDebug() << "RunDetector::setSensitivity: minStepTimeDiff_ is" << minStepTimeDiff_;
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
        } else {
            qDebug() << "x" << timeDiff;
        }
    }

    increasing_ = nowIncreasing;
    lastReading_ = reading;
    return true;
}
