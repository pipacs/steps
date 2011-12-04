#ifndef GFT_H
#define GFT_H

#include <QObject>
#include <QString>
#include <QSettings>

#define GFT_OAUTH_CONSUMER_KEY "903309545755.apps.googleusercontent.com"
#define GFT_OAUTH_CONSUMER_SECRET_KEY "bjFH7kt7nL9jrE4t8L_x7O6W"

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
    QString createRemoteDatabase(const QString archive);
    KQOAuthManager *oauthManager;
    KQOAuthRequest *oauthRequest;
    QSettings oauthSettings;
};

#endif // GFT_H
