#include "o2requestor.h"

O2Requestor::O2Requestor(QNetworkAccessManager *manager, O2 *authenticator, QObject *parent): QObject(parent) {
    manager_ = manager;
    authenticator_ = authenticator;
    // FIXME
}

O2Requestor::~O2Requestor() {
}

int O2Requestor::get(const QNetworkRequest &req) {
    // FIXME
    return -1;
}

int O2Requestor::post(const QNetworkRequest &req, const QByteArray &data) {
    // FIXME
    return -1;
}

void O2Requestor::onRefreshFinished(QNetworkReply::NetworkError error) {
    // FIXME
}
