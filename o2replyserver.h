#ifndef O2REPLYSERVER_H
#define O2REPLYSERVER_H

#include <QTcpServer>

/// HTTP server to process authentication response.
class O2ReplyServer: public QTcpServer {
    Q_OBJECT

public:
    explicit O2ReplyServer(QObject *parent = 0);
    ~O2ReplyServer();

signals:
    void verificationReceived(QMultiMap<QString, QString>);

public slots:
    void onIncomingConnection();
    void onBytesReady();
    QMultiMap<QString, QString> parseQueryParams(QByteArray *data);

public:
    QTcpSocket *socket;
};

#endif // O2REPLYSERVER_H
