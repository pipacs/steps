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
const int UPLOADER_IDLE = 1000 * 60 * 30; ///< Idle time between complete/failed uploads (ms).
const int UPLOADER_IDLE_INCOMPLETE = 1000 * 3; ///< Idle time between incomplete uploads (ms).

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
    connect(Gft::instance(), SIGNAL(uploadFinished(QString, int)), this, SLOT(onUploadFinished(QString, int)));
    connect(Qc::instance(), SIGNAL(uploadFinished(QString, int)), this, SLOT(onUploadFinished(QString, int)));
}

UploaderWorker::~UploaderWorker() {
}

void UploaderWorker::upload() {
    Trace _("UploaderWorker::upload");
    QStringList archives = listArchives();
    if (!archives.count()) {
        qDebug() << "No archives";
        emit uploadComplete(UploadComplete);
        return;
    }
    QString archive = archives[0];
    Gft::instance()->upload(archive);
    Qc::instance()->upload(archive);
}

void UploaderWorker::onUploadFinished(const QString &archive, int result) {
    Trace _("UploaderWorker::onGftUploadFinished");
    if (result == UploadComplete) {
        deleteArchiveIfUploaded(archive);
        if (listArchives().length() > 1) {
            qDebug() << "There are more archives to upload";
            result = UploadIncomplete;
        }
    }
    qDebug() << "Result" << (int)result;
    emit uploadComplete(result);
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

void UploaderWorker::deleteArchiveIfUploaded(const QString &archive) {
    Trace _("UploaderWorker::deleteArchiveIfUploaded");
    Database db(archive);

    QSqlQuery query("select count(*) from log", db.db());
    query.next();
    qlonglong total = query.value(0).toLongLong();

    query.clear();
    query.exec("select count(*) from log where inqc = 1 and ingft = 1");
    query.next();
    qlonglong totalUploaded = query.value(0).toLongLong();

    db.close();
    if (total == totalUploaded) {
        qDebug() << "All records uploaded, deleting" << archive;
        QFile(archive).remove();
    }
}
