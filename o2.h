#ifndef O2_H
#define O2_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMultiMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class O2ReplyServer;

/// Simple OAuth2 authenticator.
class O2: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool linked READ linked NOTIFY linkedChanged)

public:
    explicit O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &tokenUrl, const QUrl &refreshTokenUrl, const QUrl &redirectUrl, QObject *parent = 0);
    virtual ~O2();
    bool linked();
    QString code();
    void setCode(const QString &c);

public slots:
    Q_INVOKABLE void link();
    Q_INVOKABLE void unlink();
    void refresh();

signals:
    void openBrowser(const QUrl &url);
    void closeBrowser();
    void linkingSucceeded();
    void linkingFailed();
    void linkingTimedOut();
    void linkedChanged();

protected slots:
    void onVerificationReceived(QMultiMap<QString, QString>);
    void onTokenReplyFinished();
    void onTokenReplyError(QNetworkReply::NetworkError error);

protected:
    QString clientId_;
    QString clientSecret_;
    QString scope_;
    QUrl requestUrl_;
    QUrl tokenUrl_;
    QUrl refreshTokenUrl_;
    QUrl redirectUrl_;
    QNetworkAccessManager *manager_;
    O2ReplyServer *replyServer_;
    QString code_;
    QNetworkReply::NetworkError tokenError_;
    QNetworkReply *tokenReply_;
};

#endif // O2_H
