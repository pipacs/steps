#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>

#include "kqoauth/kqoauthmanager.h"
#include "kqoauth/kqoauthrequest.h"
#include "gftprogram.h"
#include "googledocs.h"

static const char *GFT_OAUTH_SCOPE = "https://www.googleapis.com/auth/fusiontables";

static GoogleDocs *instance_;

GoogleDocs *GoogleDocs::instance() {
    if (!instance_) {
        instance_ = new GoogleDocs();
    }
    return instance_;
}

void GoogleDocs::close() {
    delete instance_;
    instance_ = 0;
}

GoogleDocs::GoogleDocs(QObject *parent): QObject(parent) {
    oauthRequest = new KQOAuthRequest;
    oauthManager = new KQOAuthManager(this);

    oauthRequest->setEnableDebugOutput(true);
}

GoogleDocs::~GoogleDocs() {
    delete oauthRequest;
    delete oauthManager;
}

bool GoogleDocs::linked() {
    return oauthSettings.value("gd_oauth_token").toString().length() && oauthSettings.value("gd_oauth_token_secret").toString().length();
}

void GoogleDocs::link() {
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
    oauthRequest->initRequest(KQOAuthRequest::TemporaryCredentials, QUrl("https://www.google.com/accounts/OAuthGetRequestToken"));
    oauthRequest->setConsumerKey(GFT_OAUTH_CONSUMER_KEY);
    oauthRequest->setConsumerSecretKey(GFT_OAUTH_CONSUMER_SECRET_KEY);
    KQOAuthParameters parameters;
    parameters.insert("scope", GFT_OAUTH_SCOPE);
    parameters.insert("oauth_signature_method", "HMAC-SHA");
    oauthRequest->setAdditionalParameters(parameters);
    oauthManager->setHandleUserAuthorization(true);
    oauthManager->executeRequest(oauthRequest);
}

void GoogleDocs::unlink() {
    oauthSettings.setValue("gd_oauth_token", QString());
    oauthSettings.setValue("gd_oauth_token_secret", QString());
    emit linkedChanged();
}

void GoogleDocs::onAuthorizationReceived(QString token, QString verifier) {
    qDebug() << "GoogleDocs::onAuthorizationReceived: User authorization received: " << token << verifier;
    // oauthManager->getUserAccessTokens(QUrl("https://api.twitter.com/oauth/access_token"));
    oauthManager->getUserAccessTokens(QUrl("https://www.google.com//accounts/OAuthGetAccessToken"));
    if (oauthManager->lastError() != KQOAuthManager::NoError) {
        emit linkingFailed("oops");
    }
}

void GoogleDocs::onAccessTokenReceived(QString token, QString tokenSecret) {
    qDebug() << "GoogleDocs::onAccessTokenReceived: Access token received: " << token << tokenSecret;
    oauthSettings.setValue("gd_oauth_token", token);
    oauthSettings.setValue("gd_oauth_token_secret", tokenSecret);
    qDebug() << " Access tokens now stored. Authentication complete.";
    emit linkedChanged();
    emit linkingSucceeded();
}

void GoogleDocs::onAuthorizedRequestDone() {
    qDebug() << "GoogleDocs::onAuthorizedRequestDone: Request sent to Twitter!";
}

void GoogleDocs::onRequestReady(QByteArray response) {
    qDebug() << "GoogleDocs::onRequestReady: Response from the service: " << response;
}

void GoogleDocs::onTemporaryTokenReceived(QString token, QString tokenSecret) {
    qDebug() << "GoogleDocs::onTemporaryTokenReceived: Temporary token received: " << token << tokenSecret;

    QUrl userAuthURL("https://accounts.google.com/OAuthAuthorizeToken");
    if (oauthManager->lastError() == KQOAuthManager::NoError) {
        qDebug() << " Asking for user's permission to access protected resources. Opening URL: " << userAuthURL;
        oauthManager->getUserAuthorization(userAuthURL);
    }
}

bool GoogleDocs::enabled() {
    return oauthSettings.value("gd_enabled", false).toBool();
}

void GoogleDocs::setEnabled(bool v) {
    oauthSettings.setValue("gd_enabled", v);
    emit enabledChanged();
}

GoogleDocs::UploadResult GoogleDocs::upload(const QString &archive) {
    qDebug() << "GoogleDocs::upload" << archive;
    if (!enabled()) {
        // FIXME: This should be checked row by row
        qDebug() << " Not enabled";
        return GoogleDocs::UploadSucceeded;
    }

    QString token = oauthSettings.value("gd_oauth_token").toString();
    QString tokenSecret = oauthSettings.value("gd_oauth_token_secret").toString();
    if (!token.length() || !tokenSecret.length()) {
        qDebug() << " Not linked";
        return GoogleDocs::UploadCompleted;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::toNativeSeparators(archive));
    if (!db.open()) {
        qCritical() << "GoogleDocs::upload: Could not open database";
        return GoogleDocs::UploadFailed;
    }

    QSqlQuery query;
    if (!query.exec("select id, date, steps from log")) {
        qCritical() << "GoogleDocs::upload: Could not execute query:" << query.lastError().text();
        return GoogleDocs::UploadFailed;
    }

    // FIXME: Create and execute Gft program

    return GoogleDocs::UploadSucceeded;
}
