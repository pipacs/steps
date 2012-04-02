#include <QDebug>

#include "o2requestor.h"
#include "o2.h"

O2Requestor::O2Requestor(QNetworkAccessManager *manager, O2 *authenticator, QObject *parent): QObject(parent), reply_(NULL), status_(Idle) {
    manager_ = manager;
    authenticator_ = authenticator;
    connect(authenticator, SIGNAL(refreshFinished(QNetworkReply::NetworkError)), this, SLOT(onRefreshFinished(QNetworkReply::NetworkError)));
}

O2Requestor::~O2Requestor() {
}

int O2Requestor::get(const QNetworkRequest &req) {
    if (-1 == setup(req, QNetworkAccessManager::GetOperation)) {
        return -1;
    }
    reply_ = manager_->get(request_);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)));
    connect(reply_, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    return id_;
}

int O2Requestor::post(const QNetworkRequest &req, const QByteArray &data) {
    if (-1 == setup(req, QNetworkAccessManager::PostOperation)) {
        return -1;
    }
    data_ = data;
    reply_ = manager_->post(request_, data_);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)));
    connect(reply_, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    return id_;
}

void O2Requestor::onRefreshFinished(QNetworkReply::NetworkError error) {
    qDebug() << "O2Requestor::onRefreshFinished";
    if (status_ != Requesting) {
        qWarning() << "O2Reqestor::onRefreshFinished: No pending request";
        return;
    }
    if (QNetworkReply::NoError == error) {
        retry();
    } else {
        finish(error);
    }
}

void O2Requestor::onRequestFinished() {
    if (status_ == Idle) {
        qDebug() << "O2Requestor::onRequestFinished: No pending request";
        return;
    }
    if (reply_->error() == QNetworkReply::NoError) {
        finish(QNetworkReply::NoError);
    }
}

void O2Requestor::onRequestError(QNetworkReply::NetworkError error) {
    qDebug() << "O2Requestor::onRequestError: Error" << (int)error;
    if (status_ == Idle) {
        qWarning() << "O2Requestor::onRequestError: No pending request";
        return;
    }
    if (status_ == Requesting) {
        int httpStatus = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "O2Requestor::onRequestError: HTTP status" << httpStatus << reply_->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        if (httpStatus == 401) {
            // Call O2::refresh. Note the O2 instance might live in a different thread
            qDebug() << "O2Requestor::onRequestError: Refreshing token";
            if (QMetaObject::invokeMethod(authenticator_, "refresh")) {
                return;
            }
            qCritical() << "O2Requestor::onRequestError: Invoking remote refresh failed";
        }
    }
    finish(error);
}

int O2Requestor::setup(const QNetworkRequest &req, QNetworkAccessManager::Operation operation) {
    static int currentId;
    QUrl url;

    if (status_ != Idle) {
        qWarning() << "O2Requestor::setup: Another request pending";
        return -1;
    }

    request_ = req;
    operation_ = operation;
    id_ = currentId++;
    url_ = url = req.url();
    url.addQueryItem("access_token", authenticator_->token());
    request_.setUrl(url);
    status_ = Requesting;
    return id_;
}

void O2Requestor::finish(QNetworkReply::NetworkError error) {
    qDebug() << "O2Requestor::finish";
    QByteArray data;
    if (status_ == Idle) {
        qWarning() << "O2Requestor::finish: No pending request";
        return;
    }
    if (QNetworkReply::NoError == error) {
        data = reply_->readAll();
    }
    status_ = Idle;
    reply_->deleteLater();
    emit finished(id_, error, data);
}

void O2Requestor::retry() {
    qDebug() << "O2Requestor::retry";
    if (status_ != Requesting) {
        qWarning() << "O2Requestor::retry: No pending request";
        return;
    }
    reply_->deleteLater();
    QUrl url = url_;
    url_.addQueryItem("access_token", authenticator_->token());
    request_.setUrl(url);
    status_ = ReRequesting;
    if (operation_ == QNetworkAccessManager::GetOperation) {
        reply_ = manager_->get(request_);
    } else {
        reply_ = manager_->post(request_, data_);
    }
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)));
    connect(reply_, SIGNAL(finished()), this, SLOT(onRequestFinished()));
}
