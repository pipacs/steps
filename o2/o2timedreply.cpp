#include <QTimer>
#include <QNetworkReply>

#include "o2timedreply.h"

O2TimedReply::O2TimedReply(QNetworkReply *r, int timeOut, QObject *parent): QTimer(parent), reply(r) {
    setSingleShot(true);
    connect(this, SIGNAL(error(QNetworkReply::NetworkError)), reply, SIGNAL(error(QNetworkReply::NetworkError)));
    connect(this, SIGNAL(timeout()), this, SLOT(onTimeOut()));
    start(timeOut);
}

void O2TimedReply::onTimeOut() {
    emit error(QNetworkReply::TimeoutError);
}

O2TimedReplyList::~O2TimedReplyList() {
    foreach (O2TimedReply *timedReply, replies_) {
        delete timedReply;
    }
}

void O2TimedReplyList::addReply(QNetworkReply *reply) {
    O2TimedReply *timedReply = new O2TimedReply(reply);
    replies_.append(timedReply);
}

void O2TimedReplyList::removeReply(QNetworkReply *reply) {
    for (int i = 0; i < replies_.length(); i++) {
        if (replies_[i]->reply == reply) {
            delete replies_.takeAt(i);
            break;
        }
    }
}
