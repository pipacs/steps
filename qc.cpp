#include <QUrl>
#include <QSettings>

#include "qc.h"
#include "qcsecret.h"
#include "json/json.h"
#include "platform.h"
#include "trace.h"

static Qc *instance_;
static const int recordsPerUpload = 150;

Qc::Qc(QObject *parent): O1(parent) {
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

void Qc::upload(const QString &archive_) {
    Trace _("Qc::upload");
    qDebug() << "Archive:" << archive_;

    archive = archive_;
    Database db(archive);

    // Do nothing if database is empty
    {
        qlonglong total = -1;
        QSqlQuery query("select count(*) from log where ingft = 0", db.db());
        query.next();
        total = query.value(0).toLongLong();
        if (total == 0) {
            qDebug() << "Database empty";
            emit uploadFinished(archive, UploadComplete);
            return;
        }
    }

    // Mark all records as uploaded if QC is not enabled, then succeed
    if (!enabled()) {
        qDebug() << "GFT not enabled";
        QSqlQuery query("update log set ingft = 1", db.db());
        query.exec();
        emit uploadFinished(archive, UploadComplete);
        return;
    }

    // Fail if not logged in
    if (!linked()) {
        qDebug() << "Not logged in";
        emit uploadFinished(archive, UploadFailed);
        return;
    }

    QSqlQuery query(db.db());
    query.setForwardOnly(true);
    if (!query.exec("select id, date, steps from log where inqc = 0")) {
        qCritical() << "Qc::upload: Could not query archive:" << query.lastError().text();
        emit uploadFinished(archive, UploadFailed);
        return;
    }
    int numRecords = 0;
    QString sql;
    QList<qlonglong> idList;
    while (query.next()) {
        if (++numRecords > recordsPerUpload) {
            break;
        }
        qlonglong id = query.value(0).toLongLong();
        QString date = query.value(1).toString();
        int steps = query.value(2).toInt();
        QMap<QString, QString> tags = getTags(db, id);
        QString device = Platform::instance()->deviceId();

        // FIXME: ...
    }

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
        qCritical() << "Qc::getTags: Could not query archive:" << query.lastError().text();
        return ret;
    }
    while (query.next()) {
        QString name = query.value(0).toString();
        QString value = query.value(1).toString();
        ret.insert(name, value);
    }
    return ret;
}
