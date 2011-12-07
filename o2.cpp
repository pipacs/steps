#include <QList>
#include <QPair>
#include <QDebug>
#include <QTcpServer>
#include <QMultiMap>

#include "o2.h"

O2::O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &refreshUrl, QObject *parent): QObject(parent), clientId_(clientId), clientSecret_(clientSecret), scope_(scope), requestUrl_(requestUrl), refreshUrl_(refreshUrl) {
    manager_ = new QNetworkAccessManager(this);
    localServer_ = new O2ReplyServer(this);
}

O2::~O2() {
}

void O2::link() {
    qDebug() << ">O2::link";

    // Assemble intial authentication URL
    QList<QPair<QString, QString> > parameters;
    parameters.append(qMakePair("response_type", "code"));
    parameters.append(qMakePair("client_id", clientId_));
    parameters.append(qMakePair("redirect_uri", QString("http://localhost:%1").arg(O2_LOCAL_PORT)));
    parameters.append(qMakePair("scope", scope_));
    parameters.append(qMakePair("state", "begin"));

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

class O2ReplyServer: public QTcpServer {
    Q_OBJECT
public:
    explicit O2ReplyServer(QObject *parent = 0): QTcpServer(parent) {
        connect(this, SIGNAL(newConnection()), this, SLOT(onIncomingConnection()));
    }

    ~O2ReplyServer() {
    }

signals:
    void verificationReceived(QMultiMap<QString, QString>);

public slots:
    void onIncomingConnection() {
        qDebug() << ">O2ReplyServer::onIncomingConnection";
        socket = nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), this, SLOT(onBytesReady()), Qt::UniqueConnection);
        qDebug() << "<O2ReplyServer::onIncomingConnection";
    }

    void onBytesReady() {
        qDebug() << ">O2ReplyServer::onBytesReady";
        QByteArray reply;
        QByteArray content;
        content.append("<HTML></HTML>");
        reply.append("HTTP/1.0 200 OK \r\n");
        reply.append("Content-Type: text/html; charset=\"utf-8\"\r\n");
        reply.append(QString("Content-Length: %1\r\n").arg(content.size()));
        reply.append("\r\n");
        reply.append(content);
        socket->write(reply);

        QByteArray data = socket->readAll();
        QMultiMap<QString, QString> queryParams = parseQueryParams(&data);

        socket->disconnectFromHost();
        close();
        emit verificationReceived(queryParams);
        qDebug() << "<O2ReplyServer::onBytesReady";
    }

    QMultiMap<QString, QString> parseQueryParams(QByteArray *data) {
        QString splitGetLine = QString(*data).split("\r\n").first();   // Retrieve the first line with query params.
        splitGetLine.remove("GET ");                                   // Clean the line from GET
        splitGetLine.remove("HTTP/1.1");                               // From HTTP
        splitGetLine.remove("\r\n");                                   // And from rest.
        splitGetLine.prepend("http://localhost");                      // Now, make it a URL
        QUrl getTokenUrl(splitGetLine);
        QList< QPair<QString, QString> > tokens = getTokenUrl.queryItems();  // Ask QUrl to do our work.
        QMultiMap<QString, QString> queryParams;
        QPair<QString, QString> tokenPair;
        foreach (tokenPair, tokens) {
            queryParams.insert(tokenPair.first.trimmed(), tokenPair.second.trimmed());
        }
        return queryParams;
    }

public:
    QTcpSocket *socket;
};
