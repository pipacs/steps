#ifndef UPLOADER_H
#define UPLOADER_H

#include <QObject>
#include <QString>
#include <QStringList>

class QThread;
class QTimer;
class UploaderWorker;

/// Outcome of an upload.
enum UploadResult {
    UploadComplete = 0, ///< All records uploaded.
    UploadIncomplete,   ///< Some records uploaded.
    UploadFailed        ///< Upload failed.
};

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
    QTimer *uploadTimer;

protected slots:
    void onUploadComplete(int result);
};

/// Do the real uploading work.
class UploaderWorker: public QObject {
    Q_OBJECT

signals:
    void uploadComplete(int result);

public:
    explicit UploaderWorker(QObject *parent = 0);
    virtual ~UploaderWorker();

    /// Delete records where upload is complete.
    /// @return Qsee UploadResult.
    int deleteUploadedRecords();

public slots:
    /// Upload some data.
    void upload();

    /// An upload batch has finished.
    /// @param  complete    @see UploadResult.
    void onUploadFinished(int result);
};

#endif // UPLOADER_H
