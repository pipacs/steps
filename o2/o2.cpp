/*
O2: OAuth 2.0 authenticator for Qt.

Author: Akos Polster (akos@pipacs.com). Inspired by KQOAuth, the OAuth library made by Johan Paul (johan.paul@d-pointer.com).

O2 is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

O2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with O2. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QList>
#include <QPair>
#include <QDebug>
#include <QTcpServer>
#include <QMap>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QScriptEngine>
#include <QDateTime>
#include <QCryptographicHash>
#include <QTimer>

#include "o2.h"
#include "o2replyserver.h"
#include "simplecrypt.h"

O2::O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &tokenUrl, const QUrl &refreshTokenUrl, QObject *parent): QObject(parent), clientId_(clientId), clientSecret_(clientSecret), scope_(scope), requestUrl_(requestUrl), tokenUrl_(tokenUrl), refreshTokenUrl_(refreshTokenUrl) {
    QByteArray hash = QCryptographicHash::hash(clientSecret.toUtf8() + "12345678", QCryptographicHash::Sha1);
    crypt_ = new SimpleCrypt(*((quint64 *)(void *)hash.data()));
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    connect(replyServer_, SIGNAL(verificationReceived(QMap<QString,QString>)), this, SLOT(onVerificationReceived(QMap<QString,QString>)));
}

O2::~O2() {
    delete crypt_;
}

void O2::link() {
    if (linked()) {
        return;
    }

    // Start listening to authentication replies
    replyServer_->listen();

    // Save redirect URI, as we have to reuse it when requesting the access token
    redirectUri_ = QString("http://localhost:%1").arg(replyServer_->serverPort());

    // Assemble intial authentication URL
    QList<QPair<QString, QString> > parameters;
    parameters.append(qMakePair(QString("response_type"), QString("code")));
    parameters.append(qMakePair(QString("client_id"), clientId_));
    parameters.append(qMakePair(QString("redirect_uri"), redirectUri_));
    parameters.append(qMakePair(QString("scope"), scope_));

    // Show authentication URL with a web browser
    QUrl url(requestUrl_);
    url.setQueryItems(parameters);
    emit openBrowser(url);
}

void O2::unlink() {
    if (!linked()) {
        return;
    }
    setToken(QString());
    setRefreshToken(QString());
    setExpires(0);
    emit linkedChanged();
}

bool O2::linked() {
    return token().length() && (expires() > QDateTime::currentMSecsSinceEpoch() / 1000);
}

void O2::onVerificationReceived(const QMap<QString, QString> response) {
    emit closeBrowser();
    if (response.contains("error")) {
        qDebug() << "O2::onVerificationReceived: Verification failed";
        emit linkingFailed();
        return;
    }

    // Save access code
    setCode(response.value(QString("code")));

    // Exchange access code for access/refresh tokens
    QNetworkRequest tokenRequest(tokenUrl_);
    tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QMap<QString, QString> parameters;
    parameters.insert("code", code());
    parameters.insert("client_id", clientId_);
    parameters.insert("client_secret", clientSecret_);
    parameters.insert("redirect_uri", redirectUri_);
    parameters.insert("grant_type", "authorization_code");
    QByteArray data = buildRequestBody(parameters);
    tokenReply_ = manager_->post(tokenRequest, data);
    connect(tokenReply_, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()));
    connect(tokenReply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)));
}

QString O2::code() {
    QString key = QString("code.%1").arg(clientId_);
    return crypt_->decryptToString(QSettings().value(key).toString());
}

void O2::setCode(const QString &c) {
    QString key = QString("code.%1").arg(clientId_);
    QSettings().setValue(key, crypt_->encryptToString(c));
}

void O2::onTokenReplyFinished() {
    if (tokenReply_->error() == QNetworkReply::NoError) {
        QByteArray reply = tokenReply_->readAll();
        QScriptValue value;
        QScriptEngine engine;
        value = engine.evaluate("(" + QString(reply) + ")");
        setToken(value.property("access_token").toString());
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + value.property("expires_in").toInteger());
        setRefreshToken(value.property("refresh_token").toString());
        emit linkingSucceeded();
        emit tokenChanged();
        emit linkedChanged();
    }
    tokenReply_->deleteLater();
}

void O2::onTokenReplyError(QNetworkReply::NetworkError error) {
    qDebug() << "O2::onTokenReplyError" << error << tokenReply_->errorString();
    setToken(QString());
    setRefreshToken(QString());
    emit tokenChanged();
    emit linkingFailed();
    emit linkedChanged();
}

QByteArray O2::buildRequestBody(const QMap<QString, QString> &parameters) {
    QByteArray body;
    bool first = true;
    foreach (QString key, parameters.keys()) {
        if (first) {
            first = false;
        } else {
            body.append("&");
        }
        QString value = parameters.value(key);
        body.append(QUrl::toPercentEncoding(key) + QString("=").toUtf8() + QUrl::toPercentEncoding(value));
    }
    return body;
}

QString O2::token() {
    QString key = QString("token.%1").arg(clientId_);
    return crypt_->decryptToString(QSettings().value(key).toString());
}

void O2::setToken(const QString &v) {
    QString key = QString("token.%1").arg(clientId_);
    QSettings().setValue(key, crypt_->encryptToString(v));
}

int O2::expires() {
    QString key = QString("expires.%1").arg(clientId_);
    return QSettings().value(key).toInt();
}

void O2::setExpires(int v) {
    QString key = QString("expires.%1").arg(clientId_);
    QSettings().setValue(key, v);
}

QString O2::refreshToken() {
    QString key = QString("refreshtoken.%1").arg(clientId_);
    QString ret = crypt_->decryptToString(QSettings().value(key).toString());
    qDebug() << "O2::refreshToken: ..." << ret.right(7);
    return ret;
}

void O2::setRefreshToken(const QString &v) {
    QString key = QString("refreshtoken.%1").arg(clientId_);
    QSettings().setValue(key, crypt_->encryptToString(v));
}

void O2::refresh() {
    if (refreshReply_) {
        qWarning() << "O2::refresh: Earlier refresh still pending";
        return;
    }
    if (!refreshToken().length()) {
        qWarning() << "O2::refresh: No refresh token";
        return;
    }
    QNetworkRequest refreshRequest(refreshTokenUrl_);
    refreshRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QMap<QString, QString> parameters;
    parameters.insert("client_id", clientId_);
    parameters.insert("client_secret", clientSecret_);
    parameters.insert("refresh_token", refreshToken());
    parameters.insert("grant_type", "refresh_token");
    QByteArray data = buildRequestBody(parameters);
    refreshReply_ = manager_->post(refreshRequest, data);
    connect(refreshReply_, SIGNAL(finished()), this, SLOT(onRefreshFinished()));
    connect(tokenReply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRefreshFailed(QNetworkReply::NetworkError)));
}

void O2::onRefreshFinished() {
    qDebug() << "O2::onRefreshFinished";
    if (refreshReply_->error() == QNetworkReply::NoError) {
        QByteArray reply = refreshReply_->readAll();
        QScriptValue value;
        QScriptEngine engine;
        value = engine.evaluate("(" + QString(reply) + ")");
        setToken(value.property("access_token").toString());
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + value.property("expires_in").toInteger());
        setRefreshToken(value.property("refresh_token").toString());
        emit linkingSucceeded();
        emit tokenChanged();
        emit linkedChanged();

        // Retry or fail the pending request
        if (authReq_) {
            if (0 == authReq_->retries) {
                qDebug() << "O2::onRefreshFinished: Token has been refreshed, retrying pending request with new token";

                // Update the access token
                QUrl url = authReq_->url;
                url.addQueryItem("access_token", token());
                authReq_->request.setUrl(url);

                // Retry request
                authReq_->retries++;
                authReq_->reply->deleteLater();
                switch (authReq_->type) {
                case Post:
                    authReq_->reply = authReq_->manager.post(authReq_->request, authReq_->data);
                    break;
                case Get:
                default:
                    authReq_->reply = authReq_->manager.get(authReq_->request);
                }
            } else {
                // No more tries
                emit finished(authReq_->id, authReq_->reply);
                authReq_->reply->deleteLater();
                delete authReq_;
                authReq_ = 0;
            }
        } else {
            qWarning() << "O2::onRefreshFinished: No pending request";
        }
    }
    refreshReply_->deleteLater();
}

void O2::onRefreshFailed(QNetworkReply::NetworkError error) {
    qDebug() << "O2::onRefreshFailed:" << error << tokenReply_->errorString();
    setToken(QString());
    setRefreshToken(QString());
    emit tokenChanged();
    emit linkingFailed();
    emit linkedChanged();

    // Fail the pending request
    if (authReq_) {
        emit finished(authReq_->id, authReq_->reply);
        authReq_->reply->deleteLater();
        delete authReq_;
        authReq_ = 0;
   } else {
        qWarning() << "O2::onRefreshFailed: No pending request";
    }
}

O2::AuthReq::AuthReq(const QString &token, QNetworkAccessManager &m, const QNetworkRequest &r, RequestType t, const QByteArray &d):
    manager(m), request(r), type(t), data(d) {
    static int lastId;

    id = lastId++;
    retries = 0;
    reply = 0;
    url = request.url();
    url.addQueryItem("access_token", token);
    request.setUrl(url);
}

int O2::post(QNetworkAccessManager &manager, const QNetworkRequest &req, const QByteArray &data) {
    if (authReq_) {
        qDebug() << "O2::post: Earlier request still pending";
        return -1;
    }
    authReq_ = new AuthReq(token(), manager, req, Post, data);
    authReq_->reply = manager.post(req, data);
    connect(authReq_->reply, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    connect(authReq_->reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestFailed(QNetworkReply::NetworkError)));
    return authReq_->id;
}

int O2::get(QNetworkAccessManager &manager, const QNetworkRequest &req) {
    if (authReq_) {
        qDebug() << "O2::get: Earlier request still pending";
        return -1;
    }
    authReq_ = new AuthReq(token(), manager, req, Get, QByteArray());
    authReq_->reply = manager.get(req);
    connect(authReq_->reply, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    connect(authReq_->reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestFailed(QNetworkReply::NetworkError)));
    return authReq_->id;
}

void O2::onRequestFinished() {
    if (!authReq_) {
        qWarning() << "O2::onRequestFinished: No pending request";
        return;
    }
    if (authReq_->reply->error() == QNetworkReply::NoError) {
        int httpStatus = authReq_->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "O2::onRequestFinished: Success, HTTP status" << httpStatus;
        emit finished(authReq_->id, authReq_->reply);
        authReq_->reply->deleteLater();
        delete authReq_;
        authReq_ = 0;
    }
}

void O2::onRequestFailed(QNetworkReply::NetworkError error) {
    if (!authReq_) {
        qWarning() << "O2::onRequestFailed: No pending request";
        return;
    }
    int httpStatus = authReq_->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "O2::onRequestFailed: Error" << (int)error << "HTTP status" << httpStatus;
    // FIXME: If status == 401, then refresh the authentication token
    emit finished(authReq_->id, authReq_->reply);
    authReq_->reply->deleteLater();
    delete authReq_;
    authReq_ = 0;
}
