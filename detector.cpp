#include <QtGlobal>
#include <QDebug>

#include "detector.h"

#if defined(Q_OS_SYMBIAN)
const qreal MIN_READING_DIFF = 30; ///< Minimum acceleration difference.
#else
const qreal MIN_READING_DIFF = 50; ///< Minimum acceleration difference.
#endif

const qint64 MIN_WALKING_STEP_TIME_DIFF = 250; ///< Minimum time difference between steps while walking (ms).
const qint64 MIN_RUNNING_STEP_TIME_DIFF = 150; ///< Minimum time difference between steps while running (ms).

#if defined(Q_OS_SYMBIAN)
const int DATA_RATE_RUNNING = 0; ///< Accelerometer data rate for running (Hz).
#else
const int DATA_RATE_RUNNING = 20; ///< Accelerometer data rate for running (Hz).
#endif

#if defined(Q_OS_SYMBIAN)
const int DATA_RATE_WALKING = 0; ///< Accelerometer data rate for walking (Hz).
#else
const int DATA_RATE_WALKING = 10; ///< Accelerometer data rate for walking (Hz).
#endif

const qreal RUNNING_READING_LIMIT = 300; ///< Accelerations larger than this are usually caused by running.
const qint64 IDLE_TIME = 2000; ///< Set activity to Idle after this time (ms).
const int REPORTER_INTERVAL = 300; ///< How often steps are reported.

Detector::Detector(QObject *parent): QObject(parent), running_(false), runningStepTimeDiff_(MIN_RUNNING_STEP_TIME_DIFF), minReadingDiff_(MIN_READING_DIFF) {
    reset();
    reporter_ = new QTimer(this);
    reporter_->setSingleShot(false);
    reporter_->setInterval(REPORTER_INTERVAL);
    connect(reporter_, SIGNAL(timeout()), this, SLOT(report()));
}

void Detector::report() {
    if (stepCount_) {
        emit step(stepCount_);
        stepCount_ = 0;
    }
}

void Detector::init() {
    if (accelerometer_) {
        return;
    }
    accelerometer_ = new QAccelerometer(this);
    accelerometer_->connectToBackend();
    accelerometer_->setProperty("alwaysOn", true);
    // accelerometer_->setProperty("maximumReadingCount", 10);
    accelerometer_->setProperty("processAllReadings", true);

    qDebug() << "Detector::Detector: Available rates:";
    foreach (qrange range, accelerometer_->availableDataRates()) {
        qDebug() << "" << range.first << "-" << range.second;
    }
    qDebug() << "Detector::Detector: Sensor ID:" << accelerometer_->identifier();
    connect(accelerometer_, SIGNAL(readingChanged()), this, SLOT(detect()));
}

void Detector::setRunning(bool v) {
    if (v != running_) {
        init();
        if (v) {
            reset();
            accelerometer_->setDataRate(DATA_RATE_WALKING);
            accelerometer_->start();
            reporter_->start();
        } else {
            reporter_->stop();
            accelerometer_->stop();
            setActivity(Idle);
        }
        running_ = v;
        emit runningChanged(v);
    }
}

void Detector::detect() {
    QAccelerometerReading *r = accelerometer_->reading();
    qreal x = r->x();
    qreal y = r->y();
    qreal z = r->z();
    qreal reading = x * x + y * y + z * z;
    qreal readingDiff = reading - lastReading_;

    // Get the time of the reading, work around system clock going backwards
    qint64 now = (qint64)r->timestamp() / 1000;
    if (now < lastStepTime_) {
        qDebug() << "Detector::detect: Clock going backwards: Reading's time is" << now;
        lastStepTime_ = now;
    }

    // Guess current activity
    adapt(reading, now);

    // Filter out small changes
    if (qAbs(readingDiff) < minReadingDiff_) {
        return;
    }

    // Detect peak
    bool nowIncreasing = readingDiff > 0;
    if (increasing_ && !nowIncreasing) {
        // If didn't peak too early, register a step, and adapt to current activity
        qint64 timeDiff = now - lastStepTime_;
        if (timeDiff > minStepTimeDiff_) {
            ++stepCount_;
            lastStepTime_ = now;
            if (activity_ ==Idle) {
                setActivity(Walking);
            }
        }
    }

    increasing_ = nowIncreasing;
    lastReading_ = reading;
}

void Detector::adapt(qreal reading, qint64 timeStamp) {
    totalReading_ += reading;
    if (++readingCount_ % 40) {
        return;
    }

    qreal averageReading = totalReading_ / 40;
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
    increasing_ = false;
    lastReading_ = 100000.;
    lastStepTime_ = 0;
    minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
    readingCount_ = 0;
    totalReading_ = 0;
    stepCount_ = 0;
    setActivity(Idle);
}

void Detector::setActivity(Activity v) {
    if (activity_ != v) {
        activity_ = v;
        emit activityChanged((int)v);
    }
}

void Detector::setRunningStepTimeDiff(int diff) {
    if (diff <= 0) {
        diff = MIN_RUNNING_STEP_TIME_DIFF;
    }
    if (diff != runningStepTimeDiff_) {
        runningStepTimeDiff_ = diff;
        emit runningStepTimeDiffChanged(diff);
    }
}

void Detector::setMinReadingDiff(int diff) {
    if (diff <= 0) {
        diff = MIN_READING_DIFF;
    }
    if (diff != minReadingDiff_) {
        minReadingDiff_ = diff;
        emit minReadingDiffChanged(diff);
    }
}
