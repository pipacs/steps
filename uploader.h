#ifndef UPLOADER_H
#define UPLOADER_H

#include <QObject>

class UploaderWorker;

/// Upload interface.
class Uploader: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool uploading READ uploading NOTIFY uploadingChanged)

public:
    static Uploader *instance();
    static void close();
    bool uploading();

signals:
    void uploadingChanged(bool v);

public slots:
    Q_INVOKABLE void upload();

protected:
    explicit Uploader(QObject *parent = 0);
    ~Uploader();
    UploaderWorker *worker;
    QThread *workerThread;
    bool uploading_;

protected slots:
    void onUploadingChanged(bool v);
};

/// Do the real uploading work.
class UploaderWorker: public QObject {
    Q_OBJECT

public:
    explicit UploaderWorker(QObject *parent = 0);
    virtual ~UploaderWorker();

signals:
    void uploadingChanged(bool v);

public slots:
    /// Upload some data.
    void upload();

protected slots:
    void onUploadStart(); // FIXME: For testing only
};

#endif // UPLOADER_H
