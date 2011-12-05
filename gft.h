#ifndef GFT_H
#define GFT_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QSqlDatabase>

#define GFT_OAUTH_CONSUMER_KEY "903309545755.apps.googleusercontent.com"

class KQOAuthManager;
class KQOAuthRequest;

/// Google Fusion Tables account connector and uploader.
class Gft: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool linked READ linked NOTIFY linkedChanged)
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
    void linkedChanged();
    void linkingSucceeded();
    void linkingFailed(QString error);
    void openUrl(QString url);
    void enabledChanged();

public slots:
    Q_INVOKABLE void link();
    Q_INVOKABLE void unlink();
    void setEnabled(bool v);

protected slots:
    void onTemporaryTokenReceived(QString temporaryToken, QString temporaryTokenSecret);
    void onAuthorizationReceived(QString token, QString verifier);
    void onAccessTokenReceived(QString token, QString tokenSecret);
    void onAuthorizedRequestDone();
    void onRequestReady(QByteArray);

protected:
    explicit Gft(QObject *parent = 0);
    virtual ~Gft();

    /// Get tags for a log ID
    /// @return Tags as a single string of sanitized name/value pairs: "name1=value1;name2=value2;..."
    QString getTags(QSqlDatabase db, qlonglong id);

    /// Sanitize string by removing the following characters: quote, double quote, backslash, equal, semicolon.
    QString sanitize(const QString &s);

    KQOAuthManager *oauthManager;
    KQOAuthRequest *oauthRequest;
    QSettings oauthSettings;
};

#endif // GFT_H
