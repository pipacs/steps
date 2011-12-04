#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QStringList>
#include <QFileInfo>
#include <QDir>

#include "uploader.h"
#include "gft.h"
#include "platform.h"

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
    worker->enable(true);
    connect(worker, SIGNAL(uploadingChanged(bool)), this, SLOT(onUploadingChanged(bool)));
    workerThread = new QThread(this);
    worker->moveToThread(workerThread);
    workerThread->start(QThread::LowestPriority);
}

Uploader::~Uploader() {
    qDebug() << "Uploader::~Uploader";
    (void)QMetaObject::invokeMethod(worker, "enable", Q_ARG(bool, false));
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
    qDebug() << "UploaderWorker::upload";
    if (enabled) {
        emit uploadingChanged(true);
        QStringList archives = listArchives();
        qDebug() << " Archives:" << archives;
        if (archives.count()) {
            Gft::UploadResult result = Gft::instance()->upload(archives[0]);
            if (result == Gft::UploadCompleted) {
                deleteArchive(archives[0]);
            }
        }
    }
    emit uploadingChanged(false);
    QTimer::singleShot(5000, this, SLOT(upload()));
}

void UploaderWorker::enable(bool v) {
    qDebug() << "UploadWorker::enable" << v;
    enabled = v;
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

void UploaderWorker::deleteArchive(const QString archive) {
    qDebug() << "UploadWorker::deleteArchive" << archive;
    QFile file(archive);
    if (!file.remove()) {
        qCritical() << "UploadWorker::deleteArchive: Failed to delete" << archive;
    }
}
