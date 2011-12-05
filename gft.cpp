#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>

#include "kqoauth/kqoauthmanager.h"
#include "kqoauth/kqoauthrequest.h"
#include "gftprogram.h"
#include "gft.h"

static const char *GFT_OAUTH_SCOPE = "https://www.googleapis.com/auth/fusiontables";

static Gft *instance_;

Gft *Gft::instance() {
    if (!instance_) {
        instance_ = new Gft();
    }
    return instance_;
}

void Gft::close() {
    delete instance_;
    instance_ = 0;
}

Gft::Gft(QObject *parent): QObject(parent) {
    oauthRequest = new KQOAuthRequest;
    oauthManager = new KQOAuthManager(this);

    oauthRequest->setEnableDebugOutput(true);
}

Gft::~Gft() {
    delete oauthRequest;
    delete oauthManager;
}

bool Gft::linked() {
    return oauthSettings.value("gd_oauth_token").toString().length() && oauthSettings.value("gd_oauth_token_secret").toString().length();
}

void Gft::link() {
    disconnect(oauthManager, SIGNAL(temporaryTokenReceived(QString,QString)));
    disconnect(oauthManager, SIGNAL(authorizationReceived(QString,QString)));
    disconnect(oauthManager, SIGNAL(accessTokenReceived(QString,QString)));
    disconnect(oauthManager, SIGNAL(requestReady(QByteArray)));
    disconnect(oauthManager, SIGNAL(openUrl(QString)));

    connect(oauthManager, SIGNAL(temporaryTokenReceived(QString,QString)), this, SLOT(onTemporaryTokenReceived(QString, QString)));
    connect(oauthManager, SIGNAL(authorizationReceived(QString,QString)), this, SLOT( onAuthorizationReceived(QString, QString)));
    connect(oauthManager, SIGNAL(accessTokenReceived(QString,QString)), this, SLOT(onAccessTokenReceived(QString,QString)));
    connect(oauthManager, SIGNAL(requestReady(QByteArray)), this, SLOT(onRequestReady(QByteArray)));
    connect(oauthManager, SIGNAL(openUrl(QString)), this, SIGNAL(openUrl(QString)));

    // oauthRequest->initRequest(KQOAuthRequest::TemporaryCredentials, QUrl("https://api.twitter.com/oauth/request_token"));
    // oauthRequest->initRequest(KQOAuthRequest::TemporaryCredentials, QUrl("https://www.google.com/accounts/OAuthGetRequestToken"));
    oauthRequest->initRequest(KQOAuthRequest::TemporaryCredentials, QUrl("https://accounts.google.com/o/oauth2/auth"));
    oauthRequest->setConsumerKey(GFT_OAUTH_CONSUMER_KEY);
    oauthRequest->setConsumerSecretKey(GFT_OAUTH_CONSUMER_SECRET_KEY);
    KQOAuthParameters parameters;
    parameters.insert("scope", GFT_OAUTH_SCOPE);
    parameters.insert("response_type", "code");
    oauthRequest->setAdditionalParameters(parameters);
    oauthManager->setHandleUserAuthorization(true);
    oauthManager->executeRequest(oauthRequest);
}

void Gft::unlink() {
    oauthSettings.setValue("gd_oauth_token", QString());
    oauthSettings.setValue("gd_oauth_token_secret", QString());
    emit linkedChanged();
}

void Gft::onAuthorizationReceived(QString token, QString verifier) {
    qDebug() << "Gft::onAuthorizationReceived: User authorization received: " << token << verifier;
    // oauthManager->getUserAccessTokens(QUrl("https://api.twitter.com/oauth/access_token"));
    oauthManager->getUserAccessTokens(QUrl("https://www.google.com//accounts/OAuthGetAccessToken"));
    if (oauthManager->lastError() != KQOAuthManager::NoError) {
        emit linkingFailed("oops");
    }
}

void Gft::onAccessTokenReceived(QString token, QString tokenSecret) {
    qDebug() << "Gft::onAccessTokenReceived: Access token received: " << token << tokenSecret;
    oauthSettings.setValue("gd_oauth_token", token);
    oauthSettings.setValue("gd_oauth_token_secret", tokenSecret);
    qDebug() << " Access tokens now stored. Authentication complete.";
    emit linkedChanged();
    emit linkingSucceeded();
}

void Gft::onAuthorizedRequestDone() {
    qDebug() << "Gft::onAuthorizedRequestDone: Request sent to Twitter!";
}

void Gft::onRequestReady(QByteArray response) {
    qDebug() << "Gft::onRequestReady: Response from the service: " << response;
}

void Gft::onTemporaryTokenReceived(QString token, QString tokenSecret) {
    qDebug() << "Gft::onTemporaryTokenReceived: Temporary token received: " << token << tokenSecret;

    QUrl userAuthURL("https://accounts.google.com/OAuthAuthorizeToken");
    if (oauthManager->lastError() == KQOAuthManager::NoError) {
        qDebug() << " Asking for user's permission to access protected resources. Opening URL: " << userAuthURL;
        oauthManager->getUserAuthorization(userAuthURL);
    }
}

bool Gft::enabled() {
    return oauthSettings.value("gd_enabled", false).toBool();
}

void Gft::setEnabled(bool v) {
    oauthSettings.setValue("gd_enabled", v);
    emit enabledChanged();
}

Gft::UploadResult Gft::upload(const QString &archive) {
    qDebug() << "Gft::upload" << archive;
    if (!enabled()) {
        // FIXME: This should be checked row by row
        qDebug() << " Not enabled";
        return Gft::UploadSucceeded;
    }

    QString token = oauthSettings.value("gd_oauth_token").toString();
    QString tokenSecret = oauthSettings.value("gd_oauth_token_secret").toString();
    if (!token.length() || !tokenSecret.length()) {
        qDebug() << " Not linked";
        return Gft::UploadCompleted;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::toNativeSeparators(archive));
    if (!db.open()) {
        qCritical() << "Gft::upload: Could not open database";
        return Gft::UploadFailed;
    }

    // Create Gft instruction list

    QList<GftInstruction> instructions;

    QFileInfo info(archive);
    QString dbName = info.baseName();
    instructions.append(GftInstruction(GftFindTable, dbName));
    instructions.append(GftInstruction(GftCreateTableIf, dbName));

    QSqlQuery query("select id, date, steps from log", db);
    query.setForwardOnly(true);
    if (!query.exec()) {
        qDebug() << "Gft::upload: Could not query database:" << query.lastError().text();
        return Gft::UploadFailed;
    }
    QSqlRecord record = query.record();
    int idIndex = record.indexOf("id");
    int dateIndex = record.indexOf("date");
    int stepsIndex = record.indexOf("steps");
    while (query.next()) {
        qlonglong id = query.value(idIndex).toLongLong();
        QString date = sanitize(query.value(dateIndex).toString());
        int steps = query.value(stepsIndex).toInt();
        QString tags = getTags(db, id);
        GftInstruction instruction(GftQuery, QString("INSERT INTO $T (steps, date, tags) VALUES (%1, '%2', '%3')").arg(steps).arg(date).arg(tags));
        qDebug() << "" << instruction.param;
        instructions.append(instruction);
    }
    db.close();

    // Execute Gft program
    qDebug() << " Running GFT program";
    GftProgram program(0);
    program.setToken(token);
    program.setSecret(tokenSecret);
    program.setInstructions(instructions);
    program.run();
    program.wait();
    qDebug() << " GFT program finished";

    return Gft::UploadFailed; // FIXME Gft::UploadSucceeded;
}

QString Gft::getTags(QSqlDatabase db, qlonglong id) {
    QString ret;
    QSqlQuery query("select name, value from tags where logId = ?", db);
    query.bindValue(0, id);
    if (!query.exec()) {
        qDebug() << "Gft::upload: Could not query database:" << query.lastError().text();
        return ret;
    }
    QSqlRecord record = query.record();
    int nameIndex = record.indexOf("name");
    int valueIndex = record.indexOf("value");
    while (query.next()) {
        QString name = query.value(nameIndex).toString();
        QString value = query.value(valueIndex).toString();
        ret.append(sanitize(name));
        ret.append("=");
        ret.append(sanitize(value));
        ret.append(";");
    }
}

QString Gft::sanitize(const QString &s) {
    QString ret = s;
    ret.remove('\'');
    ret.remove('"');
    ret.remove(';');
    ret.remove('=');
    ret.remove('\\');
    return ret;
}
