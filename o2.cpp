#include <QList>
#include <QPair>
#include <QDebug>
#include <QTcpServer>
#include <QMultiMap>

#include "o2.h"
#include "o2replyserver.h"

O2::O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &refreshUrl, QObject *parent): QObject(parent), clientId_(clientId), clientSecret_(clientSecret), scope_(scope), requestUrl_(requestUrl), refreshUrl_(refreshUrl) {
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
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

