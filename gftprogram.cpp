#include <QDebug>

#include "gftprogram.h"
#include "kqoauth/kqoauthmanager.h"
#include "kqoauth/kqoauthrequest.h"
#include "gft.h"

GftProgram::GftProgram(QObject *parent): QThread(parent), ic(0), status(Idle), oauthRequest(0) {
    qDebug() << "GftProgram::GftProgram";
    oauthManager = new KQOAuthManager(this);
    connect(oauthManager, SIGNAL(requestReady(QByteArray)), this, SLOT(stepReady(QByteArray)));
    connect(oauthManager, SIGNAL(authorizedRequestDone()), this, SLOT(stepDone()));
}

GftProgram::~GftProgram() {
    qDebug() << "GftProgram::~GftProgram";
    delete oauthRequest;
    delete oauthManager;
}

void GftProgram::setToken(const QString &token_) {
    token = token_;
}

void GftProgram::setSecret(const QString &secret_) {
    secret = secret_;
}

void GftProgram::setInstructions(const QList<GftInstruction> instructions_) {
    instructions = instructions_;
}

void GftProgram::run() {
    qDebug() << "GftProgram::run";
    step();
    exec();
}

void GftProgram::step() {
    qDebug() << "*** GftProgram::step";

    if (status == Completed || status == Failed) {
        qCritical() << "GftProgram::step: Attempting to run a Completed or Failed program";
        quit();
        return;
    }

    if (status == Idle) {
        status = Running;
    }

    if (ic >= instructions.length()) {
        qDebug() << " No more instructions";
        status = Completed;
        quit();
        return;
    }

    // Build SQL statement based on current instruction
    QString sql;
    KQOAuthRequest::RequestHttpMethod method;
    switch (instructions[ic].op) {
    case GftFindTable:
        sql = QString("SHOW TABLES");
        method = KQOAuthRequest::GET;
        break;

    case GftCreateTableIf:
        if (!tableId.isNull()) {
            qDebug() << " GftCreateTableIf --> Table exists, skipping";
            stepReady("-");
            return;
        }
        sql = QString("CREATE TABLE '%1' (steps: NUMBER, date: STRING, tags: STRING)").arg(instructions[ic].param);
        method = KQOAuthRequest::POST;
        break;

    default:
        // At this point, we should have a table ID. If there isn't, fail.
        if (tableId.isNull()) {
            qCritical() << "GftProgram::step: No table ID";
            status = Failed;
            quit();
            return;
        }
        sql = instructions[ic].param;
        sql.replace("$T", tableId);
        method = KQOAuthRequest::POST;
    }

    qDebug() << "" << ((method == KQOAuthRequest::GET)? "GET": "POST") << sql;

    oauthRequest = new KQOAuthRequest;
    oauthRequest->setEnableDebugOutput(true);
    oauthRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl(GFT_SQL_URL));
    oauthRequest->setConsumerKey(GFT_OAUTH_CONSUMER_KEY);
    oauthRequest->setConsumerSecretKey(GFT_OAUTH_CONSUMER_SECRET_KEY);
    oauthRequest->setToken(token);
    oauthRequest->setTokenSecret(secret);
    oauthRequest->setHttpMethod(method);

    KQOAuthParameters params;
    params.insert(QString("sql"), sql);
    oauthRequest->setAdditionalParameters(params);

    oauthManager->executeRequest(oauthRequest);
}

void GftProgram::stepReady(QByteArray response) {
    qDebug() << "*** GftProgram::stepReady" << response;

    // FIXME: Interpret response, set status and errorMsg

}

void GftProgram::stepDone() {
    qDebug() << "*** GftProgram::stepDone";
    delete oauthRequest;
    oauthRequest = 0;
    ic++;
    step();
}
