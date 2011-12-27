/*
O2: OAuth 2.0 authenticator for Qt.

Author: Akos Polster (akos@pipacs.com). Inspired by KQOAuth, the OAuth library made by Johan Paul (johan.paul@d-pointer.com).

O2 is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

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

protected slots:
    /// Handle verification response.
    void onVerificationReceived(QMap<QString, QString>);

    /// Handle completion of requesing a token.
    void onTokenReplyFinished();

    /// Handle error while requesting a token.
    void onTokenReplyError(QNetworkReply::NetworkError error);

    /// Refresh token.
    void refresh();

protected:
    /// Build HTTP request body.
    QByteArray buildRequestBody(const QMap<QString, QString> &parameters);

    /// Schedule next token refresh.
    void scheduleRefresh();

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
    QTimer *refreshTimer_;
};

#endif // O2_H
