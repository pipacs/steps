/*
O2: OAuth 2.0 authenticator for Qt.

Author: Akos Polster (akos@pipacs.com). Inspired by KQOAuth, the OAuth library made by Johan Paul (johan.paul@d-pointer.com).

O2 is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

O2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with O2. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef O2_H
#define O2_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class O2ReplyServer;
class SimpleCrypt;
class QTimer;

/// Simple OAuth2 authenticator.
class O2: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool linked READ linked NOTIFY linkedChanged)
    Q_PROPERTY(QString token READ token NOTIFY tokenChanged)

public:
    /// Request type.
    enum RequestType {
        Get, Post
    };

    /// Constructor.
    /// @param  clientId        Client ID.
    /// @param  clientSecret    Client secret.
    /// @param  scope           Scope of authentication.
    /// @param  requestUrl      Authentication request target.
    /// @param  tokenUrl        Token exchange target.
    /// @param  refreshTokenUrl Token refresh target.
    /// @param  parent          Parent object.
    explicit O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &tokenUrl, const QUrl &refreshTokenUrl, QObject *parent = 0);

    /// Destructor.
    virtual ~O2();

    /// Are we authenticated?
    bool linked();

    /// Get authentication code.
    QString code();

    /// Get authentication token.
    QString token();

    /// Get refresh token.
    QString refreshToken();

    /// Get token expiration time (seconds from Epoch).
    int expires();

public slots:
    /// Authenticate.
    Q_INVOKABLE void link();

    /// De-authenticate.
    Q_INVOKABLE void unlink();

    /// Make an authenticated GET request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int get(QNetworkAccessManager &manager, const QNetworkRequest &req);

    /// Make an authenticated POST request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int post(QNetworkAccessManager &manager, const QNetworkRequest &req, const QByteArray &data);

signals:
    /// Emitted when client needs to open a web browser window, with the given URL.
    void openBrowser(const QUrl &url);

    /// Emitted when client can close the browser window.
    void closeBrowser();

    /// Emitted when authentication succeeded.
    void linkingSucceeded();

    /// Emitted when authentication failed.
    void linkingFailed();

    /// Emitted when the authentication status changed.
    void linkedChanged();

    /// Emitted when the request token changed.
    void tokenChanged();

    /// Emitted when an authenticated request finished.
    /// @param  id      Request ID, as returned by request().
    /// @param  reply   Reply. Do not delete the reply object in the slot connected to this signal. Use deleteLater().
    void finished(int id, QNetworkReply *reply);

protected slots:
    /// Handle verification response.
    void onVerificationReceived(QMap<QString, QString>);

    /// Handle completion of a token request.
    void onTokenReplyFinished();

    /// Handle failure of a token request.
    void onTokenReplyError(QNetworkReply::NetworkError error);

    /// Refresh token.
    void refresh();

    /// Handle completion of an authenticated request.
    void onRequestFinished();

    /// Handle failure of an authenticated request.
    void onRequestFailed(QNetworkReply::NetworkError error);

    /// Handle completion of a refresh request.
    void onRefreshFinished();

    /// Handle failure of a refresh request;
    void onRefreshFailed(QNetworkReply::NetworkError error);

protected:
    /// Build HTTP request body.
    QByteArray buildRequestBody(const QMap<QString, QString> &parameters);

    /// Set authentication code.
    void setCode(const QString &v);

    /// Set authentication token.
    void setToken(const QString &v);

    /// Set refresh token.
    void setRefreshToken(const QString &v);

    /// Set token expiration time.
    void setExpires(int v);

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
    QNetworkReply *tokenReply_;
    SimpleCrypt *crypt_;
    QNetworkReply *refreshReply_;

    /// Pending authenticated request.
    struct AuthReq {
        explicit AuthReq(const QString &token, QNetworkAccessManager &manager, const QNetworkRequest &request, RequestType type, const QByteArray &data);
        QNetworkAccessManager &manager;
        QNetworkRequest request;
        QNetworkReply *reply;
        RequestType type;
        int retries;
        int id;
        const QByteArray &data;
        QUrl url;
    } *authReq_;
};

#endif // O2_H
