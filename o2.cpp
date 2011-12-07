#include <QList>
#include <QPair>
#include <QDebug>
#include <QTcpServer>
#include <QMultiMap>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "o2.h"
#include "o2replyserver.h"

O2::O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &tokenUrl, const QUrl &refreshTokenUrl, const QUrl &redirectUrl, QObject *parent): QObject(parent), clientId_(clientId), clientSecret_(clientSecret), scope_(scope), requestUrl_(requestUrl), tokenUrl_(tokenUrl), refreshTokenUrl_(refreshTokenUrl), redirectUrl_(redirectUrl) {
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    connect(replyServer_, SIGNAL(verificationReceived(QMultiMap<QString,QString>)), this, SLOT(onVerificationReceived(QMultiMap<QString,QString>)));
}

O2::~O2() {
}

void O2::link() {
    qDebug() << ">O2::link";

    // Start listening to authentication replies
    replyServer_->listen();

    // Assemble intial authentication URL
    QList<QPair<QString, QString> > parameters;
    parameters.append(qMakePair(QString("response_type"), QString("code")));
    parameters.append(qMakePair(QString("client_id"), clientId_));
    parameters.append(qMakePair(QString("redirect_uri"), QString("http://localhost:%1").arg(replyServer_->serverPort())));
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
    // FIXME
}

void O2::refresh() {
    // FIXME
}

bool O2::linked() {
    // FIXME
    return false;
}

void O2::onVerificationReceived(const QMultiMap<QString, QString> response) {
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

    // Exchange access code for access token and refresh token
    QNetworkRequest tokenRequest(tokenUrl_);
    tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");


#if 0
    QByteArray data;
    data.append(QUrl::toPercentEncoding("code"));
    data.append("=");
    data.append(QUrl::toPercentEncoding(code()));
    data.append("&");

    data.append(QUrl::toPercentEncoding("client_id"));
    data.append("=");
    data.append(QUrl::toPercentEncoding(clientId_));
    data.append("&");

    data.append(QUrl::toPercentEncoding("client_secret"));
    data.append("=");
    data.append(QUrl::toPercentEncoding(clientSecret_));
    data.append("&");

    data.append(QUrl::toPercentEncoding("redirect_uri"));
    data.append("=");
    data.append("urn:ietf:wg:oauth:2.0:oob"); // QUrl::toPercentEncoding("urn:ietf:wg:oauth:2.0:oob")); // redirectUrl_.toString()));
    data.append("&");

    data.append(QUrl::toPercentEncoding("grant_type"));
    data.append("=");
    data.append(QUrl::toPercentEncoding("authorization_code"));

    data = data.toPercentEncoding("&=");
#else
    QUrl params;
    params.addQueryItem("code", code());
    params.addQueryItem("client_id", clientId_);
    params.addQueryItem("client_secret", clientSecret_);
    params.addQueryItem("redirect_uri", "http://localhost");
    params.addQueryItem("grant_type", "authorization_code");
    QByteArray data = params.encodedQuery().toPercentEncoding("&=");
#endif

    tokenError_ = QNetworkReply::NoError;
    qDebug() << " Token request URL:" << tokenRequest.url();
    qDebug() << " Token request header:";
    foreach (QByteArray header, tokenRequest.rawHeaderList()) {
        qDebug() << " " << header << tokenRequest.rawHeader(header);
    }
    qDebug() << " Token request parameters:" << data;

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
        qDebug() << "" << reply;
        emit linkingSucceeded();
    }
    tokenReply_->deleteLater();
    qDebug() << "<O2::onTokenReplyFinished";
}

void O2::onTokenReplyError(QNetworkReply::NetworkError error) {
    qDebug() << ">O2::onTokenReplyError" << error << tokenReply_->errorString();
    emit linkingFailed();
    qDebug() << "<O2::onTokenReplyError";
}
