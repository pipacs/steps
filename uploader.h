#ifndef UPLOADER_H
#define UPLOADER_H

#include <QObject>

class UploaderWorker;

/// Upload interface.
class Uploader: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool uploading READ uploading NOTIFY uploadingChanged)

signals:
    void uploadingChanged(bool changed);

public:
    static Uploader *instance();
    static void close();
    bool uploading();

public slots:
    void upload();

protected:
    explicit Uploader(QObject *parent = 0);
    ~Uploader();
    UploaderWorker *worker;
    QThread *workerThread;
    bool uploading_;

protected slots:
    void onUploadComplete();
};

/// Do the real uploading work.
class UploaderWorker: public QObject {
    Q_OBJECT

signals:
    void uploadComplete();

public:
    explicit UploaderWorker(QObject *parent = 0);
    virtual ~UploaderWorker();
    QStringList listArchives();

public slots:
    /// Upload some data.
    void upload();

    /// An upload batch to Google Fusion Tables has finished.
    /// @param  complete    True if all records in the archive have been uploaded.
    void onGftUploadFinished(bool complete);

public:
    QString archive;
};

#endif // UPLOADER_H
