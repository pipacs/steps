#ifndef O2_H
#define O2_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class O2ReplyServer;

/// Simple OAuth2 authenticator.
class O2: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool linked READ linked NOTIFY linkedChanged)
    Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)

public:
    explicit O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &tokenUrl, const QUrl &refreshTokenUrl, QObject *parent = 0);
    virtual ~O2();
    bool linked();
    QString code();
    void setCode(const QString &v);
    QString token();
    void setToken(const QString &v);
    QString refreshToken();
    void setRefreshToken(const QString &v);
    int expires();
    void setExpires(int v);

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
    void tokenChanged();

protected slots:
    void onVerificationReceived(QMap<QString, QString>);
    void onTokenReplyFinished();
    void onTokenReplyError(QNetworkReply::NetworkError error);

protected:
    QByteArray buildRequestBody(const QMap<QString, QString> &parameters);

protected:
    QString clientId_;
    QString clientSecret_;
    QString scope_;
    QUrl requestUrl_;
    QUrl tokenUrl_;
    QUrl refreshTokenUrl_;
    QString redirectUri_;
    QNetworkAccessManager *manager_;
    O2ReplyServer *replyServer_;
    QString code_;
    QNetworkReply::NetworkError tokenError_;
    QNetworkReply *tokenReply_;
};

#endif // O2_H
