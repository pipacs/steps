#ifndef O2TIMEDREPLYLIST_H
#define O2TIMEDREPLYLIST_H

#include <QList>
#include <QTimer>
#include <QNetworkReply>

class O2TimedReply: public QTimer {
    Q_OBJECT

public:
    O2TimedReply(QNetworkReply *r, int timeOut = 60000, QObject *parent = 0);

signals:
    void error(QNetworkReply::NetworkError);

protected slots:
    void onTimeOut();

public:
    QNetworkReply *reply;
};

class O2TimedReplyList {
public:
    virtual ~O2TimedReplyList();
    void addReply(QNetworkReply *reply);
    void removeReply(QNetworkReply *reply);

protected:
    QList<O2TimedReply *> replies_;
};

#endif // O2TIMEDREPLYLIST_H
