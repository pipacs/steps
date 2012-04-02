#ifndef O2REQUESTOR_H
#define O2REQUESTOR_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class O2;

/// Makes authenticated requests.
class O2Requestor: public QObject {
    Q_OBJECT

public:
    explicit O2Requestor(QNetworkAccessManager *manager, O2 *authenticator, QObject *parent = 0);
    ~O2Requestor();

public slots:
    /// Make a GET request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int get(const QNetworkRequest &req);

    /// Make a POST request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int post(const QNetworkRequest &req, const QByteArray &data);

signals:
    /// Emitted when a request has been completed or failed.
    void finished(int id, QNetworkReply::NetworkError error, QByteArray data);

protected slots:
    /// Handle refresh completion.
    void onRefreshFinished(QNetworkReply::NetworkError error);

protected:
    void setup(const QNetworkRequest *request, QNetworkAccessManager::Operation operation);
    void finish();

    QNetworkAccessManager *manager_;
    O2 *authenticator_;
    QNetworkRequest request_;
    QByteArray data_;
    QNetworkReply *reply_;
    int retries_;
    int id_;
    QNetworkAccessManager::Operation operation_;
};

#endif // O2REQUESTOR_H
