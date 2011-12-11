#ifndef GFT_H
#define GFT_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QSqlDatabase>

#include "o2/o2.h"

class GftProgram;

/// Google Fusion Tables account connector and uploader.
class Gft: public O2 {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    /// Result of an upload.
    enum UploadResult {
        UploadFailed,       ///< Upload failed.
        UploadSucceeded,    ///< Upload was successful, but there is more to upload from the archive.
        UploadCompleted     ///< Upload was successful, the complete archive has been uploaded.
    };

    static Gft *instance();
    static void close();

    /// Is upload enabled?
    bool enabled();

    /// Start uploading some records from an archive.
    void upload(const QString &archive);

signals:
    /// Emitted when enabling/disabling uploads.
    void enabledChanged();

    /// Emitted when an upload batch has finished.
    /// @param  complete    True, if all records from the archive are uploaded.
    void uploadFinished(bool complete);

public slots:
    /// Enable/disable uploads.
    void setEnabled(bool v);

protected slots:
    /// Handle step completion: Add record ID to the list of uploaded record IDs.
    void onStepCompleted(qlonglong recordId);

    /// Handle program completion.
    void onProgramCompleted();

protected:
    explicit Gft(QObject *parent = 0);
    ~Gft();

    /// Get tags for a log record ID
    /// @return Tags as a single string of sanitized name/value pairs: "name1=value1;name2=value2;..."
    QString getTags(QSqlDatabase db, qlonglong id);

    /// Sanitize string by removing the following characters: quote, double quote, backslash, equal, semicolon.
    QString sanitize(const QString &s);

protected:
    GftProgram *program;
    QList<qlonglong> uploadedRecords;
    QString archive;
};

#endif // GFT_H
