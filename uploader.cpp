#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QStringList>
#include <QFileInfo>
#include <QDir>

#include "uploader.h"
#include "gft.h"
#include "platform.h"
#include "trace.h"

static Uploader *instance_;
const int UPLOADER_IDLE = 1000 * 60 * 30;    ///< Idle time between two uploads (ms).

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
    connect(worker, SIGNAL(uploadComplete()), this, SLOT(onUploadComplete()));
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

bool Uploader::uploading() {
    return uploading_;
}

void Uploader::upload() {
    uploading_ = true;
    emit uploadingChanged(true);
    QMetaObject::invokeMethod(worker, "upload");
}

void Uploader::onUploadComplete() {
    uploading_ = false;
    emit uploadingChanged(false);
    QTimer::singleShot(UPLOADER_IDLE, this, SLOT(upload()));
}

UploaderWorker::UploaderWorker(QObject *parent): QObject(parent) {
    Gft *gft = Gft::instance();
    connect(gft, SIGNAL(uploadFinished(bool)), this, SLOT(onGftUploadFinished(bool)));
}

UploaderWorker::~UploaderWorker() {
}

void UploaderWorker::upload() {
    Trace t("UploaderWorker::upload");
    Gft *gft = Gft::instance();
    QStringList archives = listArchives();
    bool skip = false;
    if (!archives.count()) {
        qDebug() << "No archives";
        skip = true;
    } else if (!gft->enabled()) {
        qDebug() << "Uploading not enabled";
        skip = true;
    } else if (!gft->linked()) {
        qDebug() << "Not logged in";
        skip = true;
    }
    if (skip) {
        emit uploadComplete();
        return;
    }
    archive = archives[0];
    Gft::instance()->upload(archive);
}

void UploaderWorker::onGftUploadFinished(bool complete) {
    Trace t("UploaderWorker::onGftUploadFinished");
    qDebug() << "Complete?" << complete;
    if (complete) {
        QFile file(archive);
        if (!file.remove()) {
            qCritical() << "UploadWorker::onGftUploadFinished: Failed to delete" << archive;
        }
    }
    emit uploadComplete();
}

QStringList UploaderWorker::listArchives() {
    QString dbDir = QFileInfo(Platform::instance()->dbPath()).absolutePath();
    QStringList nameFilters(QString("*.adb"));
    QStringList ret;
    foreach (QString dbFile, QDir(dbDir).entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name)) {
        ret.append(dbDir + "/" + dbFile);
    }
    return ret;
}
