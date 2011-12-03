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
    QStringList listArchives();
    void deleteArchive(const QString archive);

signals:
    void uploadingChanged(bool v);

public slots:
    /// Upload some data.
    void upload();

    /// Enable/disable uploading.
    void enable(bool v);

protected:
    bool enabled;
};

#endif // UPLOADER_H
