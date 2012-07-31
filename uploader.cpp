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
#include "qc.h"

static Uploader *instance_;
const int UPLOADER_IDLE = 1000 * 60 * 60; ///< Idle time between complete/failed uploads (ms).
const int UPLOADER_IDLE_INCOMPLETE = 1000 * 60; ///< Idle time between incomplete uploads (ms).

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
    workerThread = new QThread(this);
    worker->moveToThread(workerThread);
    connect(worker, SIGNAL(uploadComplete(int)), this, SLOT(onUploadComplete(int)));
    workerThread->start(QThread::LowestPriority);
    uploadTimer = new QTimer(this);
    uploadTimer->setSingleShot(true);
    connect(uploadTimer, SIGNAL(timeout()), this, SLOT(upload()));
}

Uploader::~Uploader() {
    workerThread->quit();
    workerThread->wait();
    delete worker;
}

bool Uploader::uploading() {
    return uploading_;
}

void Uploader::upload() {
    if (uploading_) {
        return;
    }
    uploading_ = true;
    emit uploadingChanged(true);
    uploadTimer->stop();
    QMetaObject::invokeMethod(worker, "upload");
}

void Uploader::onUploadComplete(int result) {
    Trace t("Uploader::onUploadComplete");
    uploading_ = false;
    emit uploadingChanged(false);
    int timeout = UPLOADER_IDLE;
    if (result == UploadIncomplete) {
        timeout = UPLOADER_IDLE_INCOMPLETE;
    }
    uploadTimer->start(timeout);
}

UploaderWorker::UploaderWorker(QObject *parent): QObject(parent) {
    connect(Gft::instance(), SIGNAL(uploadFinished(int)), this, SLOT(onUploadFinished(int)));
    connect(Qc::instance(), SIGNAL(uploadFinished(int)), this, SLOT(onUploadFinished(int)));
}

UploaderWorker::~UploaderWorker() {
}

void UploaderWorker::upload() {
    Trace _("UploaderWorker::upload");
    Gft::instance()->upload();
    Qc::instance()->upload();
}

void UploaderWorker::onUploadFinished(int result) {
    Trace _("UploaderWorker::onUploadFinished");
    if (result == UploadComplete) {
        result = deleteUploadedRecords();
    }
    qDebug() << "Result" << (int)result;
    emit uploadComplete(result);
}

int UploaderWorker::deleteUploadedRecords() {
    QFileInfo info(Platform::instance()->dbPath());
    if (!info.exists()) {
        return UploadComplete;
    }
    if (info.size() == 0) {
        qWarning() << "UploadWorker::deleteUploadedRecords: Empty log";
        return UploadComplete;
    }

    Database db(Platform::instance()->dbPath());

    // Get total number of records
    QSqlQuery query(db.db());
    if (!query.exec("select count(*) from log")) {
        qCritical() << "UploadWorker::deleteUploadedRecords:" << query.lastError().text();
        return UploadFailed;
    }
    query.next();
    qlonglong total = query.value(0).toLongLong();

    // Get number of uploaded records
    query.clear();
    if (!query.exec("select count(*) from log where inqc = 1 and ingft = 1")) {
        qCritical() << "UploadWorker::deleteUploadedRecords:" << query.lastError().text();
        return UploadFailed;
    }
    query.next();
    qlonglong totalUploaded = query.value(0).toLongLong();

    // Delete completely uploaded records
    query.clear();
    if (!query.exec("delete from log where inqc = 1 and ingft = 1")) {
        qCritical() << "UploadWorker::deleteUploadedRecords:" << query.lastError().text();
        return UploadFailed;
    }

    if (total == totalUploaded) {
        query.clear();
        (void)query.exec("vacuum");
        return UploadComplete;
    } else {
        return UploadIncomplete;
    }
}
