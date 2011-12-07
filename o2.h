#ifndef O2_H
#define O2_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>

class O2ReplyServer;

/// Simple OAuth2 authenticator.
class O2: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool linked READ linked NOTIFY linkedChanged)

public:
    explicit O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &refreshUrl, QObject *parent = 0);
    virtual ~O2();
    bool linked();

public slots:
    void link();
    void unlink();
    void refresh();

signals:
    void openBrowser(const QUrl &url);
    void closeBrowser(const QUrl &url);
    void linkSucceeded();
    void linkFailed();
    void linkTimedOut();
    void linkedChanged();

public:
    QString clientId_;
    QString clientSecret_;
    QString scope_;
    QUrl requestUrl_;
    QUrl refreshUrl_;
    QNetworkAccessManager *manager_;
    O2ReplyServer *replyServer_;
};

#endif // O2_H
