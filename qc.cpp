#include <QUrl>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "qc.h"
#include "qcsecret.h"
#include "json/json.h"
#include "platform.h"
#include "trace.h"
#include "../o2/o1requestor.h"

using namespace QtJson;

static Qc *instance_;
static const int recordsPerUpload = 150;

Qc::Qc(QObject *parent): O1(parent) {
    manager = 0;
    requestor = 0;

    setRequestTokenUrl(QUrl(QC_REQUEST_TOKEN_URL));
    setAuthorizeUrl(QUrl(QC_AUTHORIZE_URL));
    setAccessTokenUrl(QUrl(QC_ACCESS_TOKEN_URL));
    setClientId(QC_OAUTH_CLIENT_ID);
    setClientSecret(QC_OAUTH_CLIENT_SECRET);
}

Qc::~Qc() {
}

Qc *Qc::instance() {
    if (!instance_) {
        instance_ = new Qc();
    }
    return instance_;
}

void Qc::close() {
    delete instance_;
    instance_ = 0;
}

void Qc::upload() {
    Trace _("Qc::upload");

    Database db(Platform::instance()->dbPath());

    // Do nothing if database is empty
    {
        qlonglong total = -1;
        QSqlQuery query("select count(*) from log where inqc = 0", db.db());
        query.next();
        total = query.value(0).toLongLong();
        if (total == 0) {
            qDebug() << "No records to upload";
            emit uploadFinished(UploadComplete);
            return;
        }
    }

    // Mark all records as uploaded if QC is not enabled, then succeed
    if (!enabled()) {
        qDebug() << "QC not enabled";
        QSqlQuery query("update log set ingqc = 1", db.db());
        query.exec();
        emit uploadFinished(UploadComplete);
        return;
    }

    // Fail if not logged in
    if (!linked()) {
        qDebug() << "Not logged in";
        emit uploadFinished(UploadFailed);
        return;
    }

    QSqlQuery query(db.db());
    query.setForwardOnly(true);
    if (!query.exec("select id, date, steps from log where inqc = 0")) {
        qCritical() << "Qc::upload: Could not query log:" << query.lastError().text();
        emit uploadFinished(UploadFailed);
        return;
    }

    int numRecords = 0;
    QVariantMap input;
    input["dev"] = Platform::instance()->deviceId();
    input["schema"] = "steps";
    QVariantList measurements;
    uploadedRecords.clear();

    while (query.next()) {
        if (++numRecords > recordsPerUpload) {
            break;
        }
        qlonglong id = query.value(0).toLongLong();
        uploadedRecords.append(id);
        QString date = query.value(1).toString();
        int steps = query.value(2).toInt();
        QMap<QString, QString> tags = getTags(db, id);

        QVariantMap measurement;
        measurement["at"] = QDateTime::fromString(date, Qt::ISODate).toMSecsSinceEpoch() / 1000.;
        measurement["count"] = steps;
        foreach (QString key, tags.keys()) {
            if (key == "detectedActivity") {
                QString activity = tags["detectedActivity"];
                if (activity == "0") {
                    measurement["activity"] = "idle";
                } else if (activity == "2") {
                    measurement["activity"] = "walking";
                } else {
                    measurement["activity"] = "running";
                }
            } else if (key.startsWith("x-")) {
                measurement[key] = tags[key];
            } else {
                measurement["x-" + key] = tags[key];
            }
        }
        measurements.append(measurement);
    }

    input["m"] = measurements;
    qDebug() << Json::serialize(input);

    uploadBatch(input);
}

void Qc::uploadBatch(const QVariantMap &batch) {
    Trace _("Qc::uploadBatch");

    if (!manager) {
        manager = new QNetworkAccessManager(this);
    }
    if (!requestor) {
        requestor = new O1Requestor(manager, this, this);
    }

    // Collect parameters participating in request signing
    QList<O1RequestParameter> parameters;
    parameters.append(O1RequestParameter("in", Json::serialize(batch)));

    // Add these parameters to the request body, too
    QByteArray body;
    bool first = true;
    foreach (O1RequestParameter p, parameters) {
        if (first) {
            first = false;
        } else {
            body.append("&");
        }
        body.append(QUrl::toPercentEncoding(p.name));
        body.append("=");
        body.append(QUrl::toPercentEncoding(p.value));
    }

    // Set up HTTP request
    QNetworkRequest request;
    request.setUrl(QUrl(QC_INPUT_URL));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentLengthHeader, body.length());

    // ...And post it
    QNetworkReply *reply = requestor->post(request, parameters, body);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
}

void Qc::onError(QNetworkReply::NetworkError error) {
    Trace _("Qc::onError");
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    qDebug() << error << reply->errorString();
    finishBatch(true);
}

void Qc::onFinished() {
    Trace _("Qc::onFinished");
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    if (reply->error() == QNetworkReply::NoError) {
        finishBatch(false);
    }
    reply->deleteLater();
}

bool Qc::enabled() {
    return QSettings().value("qc.enabled", false).toBool();
}

void Qc::setEnabled(bool v) {
    QSettings().setValue("qc.enabled", v);
    emit enabledChanged();
}

QMap<QString, QString> Qc::getTags(Database &db, qlonglong id) {
    QMap<QString, QString> ret;
    QSqlQuery query(db.db());
    query.prepare("select name, value from tags where logId = ?");
    query.bindValue(0, id);
    if (!query.exec()) {
        qCritical() << "Qc::getTags: Could not query log:" << query.lastError().text();
        return ret;
    }
    while (query.next()) {
        QString name = query.value(0).toString();
        QString value = query.value(1).toString();
        ret.insert(name, value);
    }
    return ret;
}

void Qc::finishBatch(bool failed) {
    Trace _("Qc::finishBatch");

    if (failed) {
        qDebug() << "Result: Failed";
        emit uploadFinished(UploadFailed);
        return;
    }

    // Mark uploaded records in the log
    Database db(Platform::instance()->dbPath());
    db.transaction();
    foreach (qlonglong id, uploadedRecords) {
        QSqlQuery query(db.db());
        query.prepare("update log set inqc = 1 where id = ?");
        query.bindValue(0, id);
        query.exec();
    }
    db.commit();

    // Determine upload result: Are there any records left in the log?
    int result = UploadIncomplete;
    qlonglong total = -1;
    QSqlQuery query("select count(*) from log where inqc = 0", db.db());
    query.next();
    total = query.value(0).toLongLong();
    qDebug() << total << "records left";
    if (total == 0) {
        qDebug() << "Result: Complete";
        result = UploadComplete;
    } else {
        qDebug() << "Result: Incomplete";
        result = UploadIncomplete;
    }
    emit uploadFinished(result);
}
