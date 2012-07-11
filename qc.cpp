#include <QUrl>
#include <QSettings>

#include "qc.h"
#include "qcsecret.h"

static Qc *instance_;

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

void Qc::upload(const QString &archive) {
}

bool Qc::enabled() {
    return QSettings().value("qc.enabled", false).toBool();
}

void Qc::setEnabled(bool v) {
    QSettings().setValue("qc.enabled", v);
    emit enabledChanged();
}
