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

#include "o2.h"
#include "o2replyserver.h"

O2::O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &tokenUrl, const QUrl &refreshTokenUrl, QObject *parent): QObject(parent), clientId_(clientId), clientSecret_(clientSecret), scope_(scope), requestUrl_(requestUrl), tokenUrl_(tokenUrl), refreshTokenUrl_(refreshTokenUrl) {
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    connect(replyServer_, SIGNAL(verificationReceived(QMap<QString,QString>)), this, SLOT(onVerificationReceived(QMap<QString,QString>)));
}

O2::~O2() {
}

void O2::link() {
    qDebug() << ">O2::link";

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
    parameters.append(qMakePair(QString("state"), QString("openbrowser")));

    // Show authentication URL with a web browser
    QUrl url(requestUrl_);
    url.setQueryItems(parameters);
    qDebug() << " Open" << url;
    emit openBrowser(url);

    qDebug() << "<O2::link";
}

void O2::unlink() {
    setToken(QString());
    setExpires(0);
    emit linkedChanged();
}

void O2::refresh() {
    // FIXME
}

bool O2::linked() {
    return token().length() && (expires() > QDateTime::currentMSecsSinceEpoch() / 1000);
}

void O2::onVerificationReceived(const QMap<QString, QString> response) {
    qDebug() << ">O2::onVerificationReceived";
    emit closeBrowser();
    if (response.contains("error")) {
        qDebug() << " Verification failed";
        emit linkingFailed();
        qDebug() << "<O2::onVerificationReceived";
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
    tokenError_ = QNetworkReply::NoError;
    tokenReply_ = manager_->post(tokenRequest, data);
    connect(tokenReply_, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()));
    connect(tokenReply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)));

    qDebug() << "<O2::onVerificationReceived";
}

QString O2::code() {
    QString key = QString("code.%1").arg(clientId_);
    return QSettings().value(key).toString();
}

void O2::setCode(const QString &c) {
    QString key = QString("code.%1").arg(clientId_);
    QSettings().setValue(key, c);
}

void O2::onTokenReplyFinished() {
    qDebug() << ">O2::onTokenReplyFinished";
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
    qDebug() << "<O2::onTokenReplyFinished";
}

void O2::onTokenReplyError(QNetworkReply::NetworkError error) {
    qDebug() << ">O2::onTokenReplyError" << error << tokenReply_->errorString();
    emit linkingFailed();
    qDebug() << "<O2::onTokenReplyError";
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
    return QSettings().value(key).toString();
}

void O2::setToken(const QString &v) {
    QString key = QString("token.%1").arg(clientId_);
    QSettings().setValue(key, v);
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
    return QSettings().value(key).toString();
}

void O2::setRefreshToken(const QString &v) {
    QString key = QString("refreshtoken.%1").arg(clientId_);
    QSettings().setValue(key, v);
}
