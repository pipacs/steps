#include <QtGlobal>
#include <QDebug>

#include "detector.h"

#if defined(Q_OS_SYMBIAN)
const qreal MIN_READING_DIFF = 20; ///< Minimum acceleration difference.
#else
const qreal MIN_READING_DIFF = 50; ///< Minimum acceleration difference.
#endif

const qint64 MIN_WALKING_STEP_TIME_DIFF = 301; ///< Minimum time difference between steps while walking (ms).
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

Detector::Detector(QObject *parent): QObject(parent), activity_(Idle), minReadingDiff_(MIN_READING_DIFF), running_(false), runningStepTimeDiff_(MIN_WALKING_STEP_TIME_DIFF) {
    worker_ = new DetectorWorker();
    workerThread_ = new QThread(this);
    worker_->moveToThread(workerThread_);

    connect(worker_, SIGNAL(activityChanged(int)), this, SLOT(onActivityChanged(int)));
    connect(worker_, SIGNAL(minReadingDiffChanged(int)), this, SLOT(onMinReadingDiffChanged(int)));
    connect(worker_, SIGNAL(runningChanged(bool)), this, SLOT(onRunningChanged(bool)));
    connect(worker_, SIGNAL(runningStepTimeDiffChanged(int)), this, SLOT(onRunningStepTimeDiffChanged(int)));
    connect(worker_, SIGNAL(step()), this, SIGNAL(step()));

    workerThread_->start(QThread::LowestPriority);
}

Detector::~Detector() {
    workerThread_->quit();
    workerThread_->wait();
    delete worker_;
}

void Detector::onActivityChanged(int value) {
    activity_ = value;
    emit activityChanged(value);
}

void Detector::onMinReadingDiffChanged(int value) {
    minReadingDiff_ = value;
    emit minReadingDiffChanged(value);
}

void Detector::onRunningChanged(bool value) {
    running_ = value;
    emit runningChanged(value);
}

void Detector::onRunningStepTimeDiffChanged(int value) {
    runningStepTimeDiff_ = value;
    emit runningStepTimeDiffChanged(value);
}

void Detector::setRunning(bool value) {
    qDebug() << "Detector::setRunning" << value;
    running_ = value;
    if (!QMetaObject::invokeMethod(
            worker_,
            "setRunning",
            Q_ARG(bool, value))) {
        qCritical() << "Detector::setRunning: Invoking remote detector failed";
    }
}

void Detector::setMinReadingDiff(int value) {
    minReadingDiff_ = value;
    if (!QMetaObject::invokeMethod(
            worker_,
            "setMinReadingDiff",
            Q_ARG(int, value))) {
        qCritical() << "Detector::setsetMinReadingDiff: Invoking remote detector failed";
    }
}

void Detector::setRunningStepTimeDiff(int value) {
    runningStepTimeDiff_ = value;
    if (!QMetaObject::invokeMethod(
            worker_,
            "setRunningStepTimeDiff",
            Q_ARG(int, value))) {
        qCritical() << "Detector::setRunningStepTimeDiff: Invoking remote detector failed";
    }
}

DetectorWorker::DetectorWorker(QObject *parent): QObject(parent), running_(false), runningStepTimeDiff_(MIN_RUNNING_STEP_TIME_DIFF), minReadingDiff_(MIN_READING_DIFF) {
    reset();
}

void DetectorWorker::init() {
    if (accelerometer_) {
        return;
    }
    accelerometer_ = new QAccelerometer(this);
    accelerometer_->connectToBackend();
    accelerometer_->setProperty("alwaysOn", true);
#if defined(Q_OS_SYMBIAN)
    accelerometer_->setProperty("maximumReadingCount", 10);
    // accelerometer_->setProperty("processAllReadings", true);
#endif
    qDebug() << "Detector::Detector: Available rates:";
    foreach (qrange range, accelerometer_->availableDataRates()) {
        qDebug() << range.first << "-" << range.second;
    }
    qDebug() << "Detector::Detector: Sensor ID" << accelerometer_->identifier();
    connect(accelerometer_, SIGNAL(readingChanged()), this, SLOT(detect()));
}

void DetectorWorker::setRunning(bool v) {
    qDebug() << "DetectorWorker::setRunning" << v;
    if (v != running_) {
        init();
        if (v) {
            qDebug() << " reset";
            reset();
            qDebug() << " setDataRate";
            accelerometer_->setDataRate(DATA_RATE_WALKING);
            qDebug() << " start";
            accelerometer_->start();
        } else {
            accelerometer_->stop();
            setActivity(Detector::Idle);
        }
        running_ = v;
        qDebug() << " emit runningChanged";
        emit runningChanged(v);
    }
}

void DetectorWorker::detect() {
    QAccelerometerReading *r = accelerometer_->reading();
    qreal x = r->x();
    qreal y = r->y();
    qreal z = r->z();
    qreal reading = x * x + y * y + z * z;
    qreal readingDiff = reading - lastReading_;

    // Get the time of the reading, work around system clock going backwards
    qint64 now = (qint64)r->timestamp() / 1000;
    if (now < lastStepTime_) {
        qDebug() << "Detector::detect: Clock going backwards";
        lastStepTime_ = now;
    }

    // Maintain average, guess current activity
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
            qDebug() << "+" << timeDiff << stepCount_;
            lastStepTime_ = now;
            if (activity_ == Detector::Idle) {
                setActivity(Detector::Walking);
            }
            emit step();
        } else {
            qDebug() << "- t" << timeDiff << "<" << minStepTimeDiff_;
        }
    }

    increasing_ = nowIncreasing;
    lastReading_ = reading;
}

void DetectorWorker::adapt(qreal reading, qint64 timeStamp) {
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
        if (activity_ != Detector::Running) {
            minStepTimeDiff_ = runningStepTimeDiff_;
            qDebug() << "Detector::adapt: Running, setting data rate to" << DATA_RATE_RUNNING;
            accelerometer_->stop();
            accelerometer_->setDataRate(DATA_RATE_RUNNING);
            accelerometer_->start();
        }
    } else {
        if (activity_ == Detector::Running) {
            minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
            qDebug() << "Detector::adapt: Walking or Idle, setting data rate to" << DATA_RATE_WALKING;
            accelerometer_->stop();
            accelerometer_->setDataRate(DATA_RATE_WALKING);
            accelerometer_->start();
        }
    }

    setActivity(isStepping? (isRunning? Detector::Running: Detector::Walking): Detector::Idle);
}

void DetectorWorker::reset() {
    increasing_ = false;
    lastReading_ = 100000.;
    lastStepTime_ = 0;
    minStepTimeDiff_ = MIN_WALKING_STEP_TIME_DIFF;
    readingCount_ = 0;
    totalReading_ = 0;
    stepCount_ = 0;
    setActivity(Detector::Idle);
}

void DetectorWorker::setActivity(Detector::Activity v) {
    if (activity_ != v) {
        activity_ = v;
        emit activityChanged((int)v);
    }
}

void DetectorWorker::setRunningStepTimeDiff(int diff) {
    if (diff <= 0) {
        diff = MIN_RUNNING_STEP_TIME_DIFF;
    }
    if (diff != runningStepTimeDiff_) {
        runningStepTimeDiff_ = diff;
        emit runningStepTimeDiffChanged(diff);
    }
}

void DetectorWorker::setMinReadingDiff(int diff) {
    if (diff <= 0) {
        diff = MIN_READING_DIFF;
    }
    if (diff != minReadingDiff_) {
        minReadingDiff_ = diff;
        emit minReadingDiffChanged(diff);
    }
}
