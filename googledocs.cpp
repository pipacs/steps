#include <QDebug>

#include "kqoauth/kqoauthmanager.h"
#include "kqoauth/kqoauthrequest.h"

#include "googledocs.h"

/* Twitter demo keys:
static const char *OAUTH_CONSUMER_KEY = "9PqhX2sX7DlmjNJ5j2Q";
static const char *OAUTH_CONSUMER_SECRET_KEY = "1NYYhpIw1fXItywS9Bw6gGRmkRyF9zB54UXkTGcI8";
static const char *OAUTH_REQUEST_TOKEN_URL = "https://api.twitter.com/oauth/request_token";
static const char *OAUTH_GET_ACCESS_URL = "https://api.twitter.com/oauth/access_token";
*/
static const char *OAUTH_CONSUMER_KEY = "903309545755.apps.googleusercontent.com";
static const char *OAUTH_CONSUMER_SECRET_KEY = "bjFH7kt7nL9jrE4t8L_x7O6W";
static const char *OAUTH_SCOPE = "https://www.googleapis.com/auth/fusiontables";

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
    return oauthSettings.value("oauth_token").toString().length() && oauthSettings.value("oauth_token_secret").toString().length();
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
    oauthRequest->setConsumerKey(OAUTH_CONSUMER_KEY);
    oauthRequest->setConsumerSecretKey(OAUTH_CONSUMER_SECRET_KEY);
    KQOAuthParameters parameters;
    parameters.insert("scope", OAUTH_SCOPE);
    parameters.insert("oauth_signature_method", "HMAC-SHA");
    oauthRequest->setAdditionalParameters(parameters);
    oauthManager->setHandleUserAuthorization(true);
    oauthManager->executeRequest(oauthRequest);
}

void GoogleDocs::unlink() {
    oauthSettings.setValue("oauth_token", QString());
    oauthSettings.setValue("oauth_token_secret", QString());
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
    oauthSettings.setValue("oauth_token", token);
    oauthSettings.setValue("oauth_token_secret", tokenSecret);
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

    // QUrl userAuthURL("https://api.twitter.com/oauth/authorize");
    QUrl userAuthURL("https://accounts.google.com/OAuthAuthorizeToken");

    if (oauthManager->lastError() == KQOAuthManager::NoError) {
        qDebug() << " Asking for user's permission to access protected resources. Opening URL: " << userAuthURL;
        oauthManager->getUserAuthorization(userAuthURL);
    }
}
