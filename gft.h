#ifndef GFT_H
#define GFT_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QSqlDatabase>

#include "o2.h"

/// Google Fusion Tables account connector and uploader.
class Gft: public O2 {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    static Gft *instance();
    static void close();
    bool linked();
    bool enabled();

    /// Result of an upload.
    enum UploadResult {
        UploadFailed,       ///< Upload failed.
        UploadSucceeded,    ///< Upload was successful, but there is more to upload from the archive.
        UploadCompleted     ///< Upload was successful, the complete archive has been uploaded.
    };

    /// Upload (parts of) an archive.
    UploadResult upload(const QString &archive);

signals:
    void enabledChanged();

public slots:
    void setEnabled(bool v);

protected:
    explicit Gft(QObject *parent = 0);
    virtual ~Gft();

    /// Get tags for a log ID
    /// @return Tags as a single string of sanitized name/value pairs: "name1=value1;name2=value2;..."
    QString getTags(QSqlDatabase db, qlonglong id);

    /// Sanitize string by removing the following characters: quote, double quote, backslash, equal, semicolon.
    QString sanitize(const QString &s);

    QSettings oauthSettings;
};

#endif // GFT_H
