#include <QDebug>

#include "gftprogram.h"
#include "kqoauth/kqoauthmanager.h"
#include "kqoauth/kqoauthrequest.h"
#include "googledocs.h"

GftProgram::GftProgram(QObject *parent, const QString &token_, const QString &secret_, const QList<GftInstruction> &program_): QObject(parent), token(token_), secret(secret_), program(program_), ic(0), status(Idle), oauthRequest(0) {
    qDebug() << "GftProgram::GftProgram";
    oauthManager = new KQOAuthManager(this);
    connect(oauthManager, SIGNAL(requestReady(QByteArray)), this, SLOT(stepReady(QByteArray)));
    connect(oauthManager, SIGNAL(authorizedRequestDone()), this, SLOT(stepDone()));
}

GftProgram::~GftProgram() {
    delete oauthRequest;
    delete oauthManager;
}

void GftProgram::step() {
    qDebug() << "GftProgram::step";
    if (status == Completed || status == Failed) {
        qCritical() << " Attempting to run a Completed or Failed program";
        return;
    }
    if (status == Idle) {
        status = Running;
    }
    if (ic == program.length()) {
        status = Completed;
        emit completed(status);
    }

    // Build SQL statement based on current instruction
    QString sql;
    // FIXME

    oauthRequest = new KQOAuthRequest;
    oauthRequest->setEnableDebugOutput(true);
    oauthRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl(GFT_SQL_URL));
    oauthRequest->setConsumerKey(GFT_OAUTH_CONSUMER_KEY);
    oauthRequest->setConsumerSecretKey(GFT_OAUTH_CONSUMER_SECRET_KEY);
    oauthRequest->setToken(token);
    oauthRequest->setTokenSecret(secret);

    KQOAuthParameters params;
    params.insert(QString("sql"), sql);
    oauthRequest->setAdditionalParameters(params);

    oauthManager->executeRequest(oauthRequest);
}

void GftProgram::stepReady(QByteArray response) {
    qDebug() << "GftProgram::stepReady" << response;
    // FIXME: Interpret response, set status and errorMsg
    delete oauthRequest;
    oauthRequest = 0;
    step();
}

void GftProgram::stepDone() {
    qDebug() << "GftProgram::stepDone";
}
