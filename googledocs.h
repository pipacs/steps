#ifndef GOOGLEDOCS_H
#define GOOGLEDOCS_H

#include <QObject>
#include <QString>
#include <QSettings>

class KQOAuthManager;
class KQOAuthRequest;

/// Link application to a Google Docs account
class GoogleDocs: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool linked READ linked NOTIFY linkedChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    static GoogleDocs *instance();
    static void close();
    bool linked();
    bool enabled();

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
    explicit GoogleDocs(QObject *parent = 0);
    virtual ~GoogleDocs();
    KQOAuthManager *oauthManager;
    KQOAuthRequest *oauthRequest;
    QSettings oauthSettings;
};

#endif // GOOGLEDOCS_H
