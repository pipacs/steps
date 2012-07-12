#ifndef QC_H
#define QC_H

#include <QMap>
#include <QString>

#include "../o2/o1.h"
#include "uploader.h"
#include "database.h"

/// QC connector and uploader.
class Qc: public O1 {
    Q_OBJECT

public:
    explicit Qc(QObject *parent = 0);
    ~Qc();

    /// Is uploading enabled?
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    bool enabled();
    void setEnabled(bool v);

    static Qc *instance();
    static void close();

    /// Start uploading some records from an archive.
    void upload(const QString &archive);

    /// Get tags.
    QMap<QString, QString> getTags(Database &db, qlonglong recordId);

signals:
    /// Uploading has been enabled or disabled.
    void enabledChanged();

    /// Upload has finished.
    /// @param  archive Database archive file name.
    /// @param  result  @see UploadResult.
    void uploadFinished(const QString &archive, int result);

public:
    QString archive;
};

#endif // QC_H
