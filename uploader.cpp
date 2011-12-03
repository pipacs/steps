#include <QTimer>
#include <QThread>
#include <QDebug>

#include "uploader.h"
#include "googledocs.h"

static Uploader *instance_;

Uploader *Uploader::instance() {
    if (!instance_) {
        instance_ = new Uploader(0);
    }
    return instance_;
}

void Uploader::close() {
    delete instance_;
    instance_ = 0;
}

Uploader::Uploader(QObject *parent): QObject(parent), uploading_(false) {
    worker = new UploaderWorker();
    connect(worker, SIGNAL(uploadingChanged(bool)), this, SLOT(onUploadingChanged(bool)));
    workerThread = new QThread(this);
    worker->moveToThread(workerThread);
    workerThread->start(QThread::LowestPriority);
}

Uploader::~Uploader() {
    qDebug() << "Uploader::~Uploader";
    workerThread->quit();
    workerThread->wait();
    delete worker;
}

void Uploader::onUploadingChanged(bool v) {
    uploading_ = v;
    emit uploadingChanged(uploading_);
}

bool Uploader::uploading() {
    return uploading_;
}

void Uploader::upload() {
    if (!QMetaObject::invokeMethod(worker, "upload")) {
        qCritical() << "Uploader::upload: Invoking UploaderWorker::upload() failed";
    }
}

UploaderWorker::UploaderWorker(QObject *parent): QObject(parent) {
}

UploaderWorker::~UploaderWorker() {
}

void UploaderWorker::upload() {
    qDebug() << "UploaderWorker: Upload stopped";
    emit uploadingChanged(false);
    QTimer::singleShot(1000, this, SLOT(onUploadStart()));
}

void UploaderWorker::onUploadStart() {
    qDebug() << "UploaderWorker: Upload started";
    emit uploadingChanged(true);
    QTimer::singleShot(3000, this, SLOT(upload()));
}
