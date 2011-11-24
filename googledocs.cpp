#include <QDebug>

#include "kqoauth/kqoauthmanager.h"
#include "kqoauth/kqoauthrequest.h"

#include "googledocs.h"

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

void GoogleDocs::link(const QString &name, const QString &password) {
    connect(oauthManager, SIGNAL(temporaryTokenReceived(QString,QString)), this, SLOT(onTemporaryTokenReceived(QString, QString)));
    connect(oauthManager, SIGNAL(authorizationReceived(QString,QString)), this, SLOT( onAuthorizationReceived(QString, QString)));
    connect(oauthManager, SIGNAL(accessTokenReceived(QString,QString)), this, SLOT(onAccessTokenReceived(QString,QString)));
    connect(oauthManager, SIGNAL(requestReady(QByteArray)), this, SLOT(onRequestReady(QByteArray)));

    oauthRequest->initRequest(KQOAuthRequest::TemporaryCredentials, QUrl("https://api.twitter.com/oauth/request_token"));
    oauthRequest->setConsumerKey("9PqhX2sX7DlmjNJ5j2Q");
    oauthRequest->setConsumerSecretKey("1NYYhpIw1fXItywS9Bw6gGRmkRyF9zB54UXkTGcI8");
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
    oauthManager->getUserAccessTokens(QUrl("https://api.twitter.com/oauth/access_token"));
    if (oauthManager->lastError() != KQOAuthManager::NoError) {
        emit linkingFailed("oops");
    }
}

void GoogleDocs::onAccessTokenReceived(QString token, QString tokenSecret) {
    qDebug() << "GoogleDocs::onAccessTokenReceived: Access token received: " << token << tokenSecret;
    oauthSettings.setValue("oauth_token", token);
    oauthSettings.setValue("oauth_token_secret", tokenSecret);
    emit linkedChanged();
    emit linkingSucceeded();
    qDebug() << "Access tokens now stored. You are ready to send Tweets from user's account!";
}

void GoogleDocs::onAuthorizedRequestDone() {
    qDebug() << "GoogleDocs::onAuthorizedRequestDone: Request sent to Twitter!";
}

void GoogleDocs::onRequestReady(QByteArray response) {
    qDebug() << "GoogleDocs::onRequestReady: Response from the service: " << response;
}

void GoogleDocs::onTemporaryTokenReceived(QString token, QString tokenSecret) {
    qDebug() << "GoogleDocs::onTemporaryTokenReceived: Temporary token received: " << token << tokenSecret;

    QUrl userAuthURL("https://api.twitter.com/oauth/authorize");

    if (oauthManager->lastError() == KQOAuthManager::NoError) {
        qDebug() << "Asking for user's permission to access protected resources. Opening URL: " << userAuthURL;
        oauthManager->getUserAuthorization(userAuthURL);
    }
}
