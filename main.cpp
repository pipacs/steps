#include <QtGui/QApplication>
#include <QtDeclarative>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QNetworkDiskCache>
#include <QTimer>

#include "qmlapplicationviewer.h"
#include "counter.h"
#include "preferences.h"
#include "platform.h"
#include "mediakey.h"
#include "logger.h"
#include "gft.h"
#include "sipfixer.h"
#include "uploader.h"
#include "trace.h"

class PersistentCookieJar: public QNetworkCookieJar {
public:
    PersistentCookieJar(QObject *parent): QNetworkCookieJar(parent) {load();}
    ~PersistentCookieJar() {save();}

    virtual QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const {
        QMutexLocker lock(&mutex);
        return QNetworkCookieJar::cookiesForUrl(url);
    }

    virtual bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url) {
        QMutexLocker lock(&mutex);
        return QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
    }

private:
    void save() {
        QMutexLocker lock(&mutex);
        QList<QNetworkCookie> list = allCookies();
        QByteArray data;
        foreach (QNetworkCookie cookie, list) {
            if (!cookie.isSessionCookie()) {
                data.append(cookie.toRawForm());
                data.append("\n");
            }
        }
        QSettings settings;
        settings.setValue("Cookies", data);
    }

    void load() {
        Trace t("PersistentCookieJar::load");
        QMutexLocker lock(&mutex);
        QSettings settings;
        QByteArray data = settings.value("Cookies").toByteArray();
        setAllCookies(QNetworkCookie::parseCookies(data));
    }

    mutable QMutex mutex;
};

class NetworkAccessManagerFactory: public QDeclarativeNetworkAccessManagerFactory {
public:
    NetworkAccessManagerFactory() {}
    ~NetworkAccessManagerFactory() {}

    QNetworkAccessManager *create(QObject *parent);
    static PersistentCookieJar *cookieJar;
    QMutex mutex;
};

PersistentCookieJar *NetworkAccessManagerFactory::cookieJar = 0;

static void cleanupCookieJar() {
    Trace t("cleanupCookieJar");
    delete NetworkAccessManagerFactory::cookieJar;
    NetworkAccessManagerFactory::cookieJar = 0;
}

QNetworkAccessManager *NetworkAccessManagerFactory::create(QObject *parent) {
    Trace t("NetworkAccessManagerFactory::create");
    QMutexLocker lock(&mutex);
    QNetworkAccessManager *manager = new QNetworkAccessManager(parent);
    if (!cookieJar) {
        qAddPostRoutine(cleanupCookieJar);
        cookieJar = new PersistentCookieJar(0);
    }
    manager->setCookieJar(cookieJar);
    cookieJar->setParent(0);

    QNetworkDiskCache *cache = new QNetworkDiskCache;
    cache->setCacheDirectory(QDir::tempPath() + QLatin1String("/qml-webview-network-cache"));
    cache->setMaximumCacheSize(5000);
    manager->setCache(cache);

    qDebug() << "Created new network access manager for" << parent;
    return manager;
}

int main(int argc, char *argv[]) {
    // Create application
    QApplication app(argc, argv);
    app.setApplicationName("Steps");
    app.setOrganizationDomain("pipacs.com");
    app.setOrganizationName("pipacs.com");
    app.setApplicationVersion(Platform::instance()->appVersion());

    qDebug() << "Steps" << Platform::instance()->appVersion() << "starting up";

    // Set up tracing
    qInstallMsgHandler(Trace::messageHandler);

    // Create singletons
    qDebug() << "Create singletons";
    Preferences *prefs = Preferences::instance();
    Platform *platform = Platform::instance();
    Logger *logger = Logger::instance();
    Gft *gft = Gft::instance();
    SipFixer *sipFixer = SipFixer::instance();
    NetworkAccessManagerFactory *namFactory = new NetworkAccessManagerFactory;
    Uploader *uploader = Uploader::instance();
    QTimer::singleShot(30000, uploader, SLOT(upload()));

    // Set up and show QML viewer
    qDebug() << "Set up QML viewer";
    qDebug() << " Viewer";
    QmlApplicationViewer *viewer = new QmlApplicationViewer;
    qDebug() << " Counter";
    Counter *counter = new Counter(viewer);
    qDebug() << " MediaKey";
    MediaKey *mediaKey = new MediaKey(viewer);
    qDebug() << " NetworkAccessManagerFactory";
    viewer->engine()->setNetworkAccessManagerFactory(namFactory);
    qDebug() << " ScreenOrientation";
    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    qDebug() << " counter";
    viewer->rootContext()->setContextProperty("counter", counter);
    qDebug() << " prefs";
    viewer->rootContext()->setContextProperty("prefs", prefs);
    qDebug() << " platform";
    viewer->rootContext()->setContextProperty("platform", platform);
    qDebug() << " logger";
    viewer->rootContext()->setContextProperty("logger", logger);
    qDebug() << " mediaKey";
    viewer->rootContext()->setContextProperty("mediaKey", mediaKey);
    qDebug() << " gft";
    viewer->rootContext()->setContextProperty("gft", gft);
    qDebug() << " sipFixer";
    viewer->rootContext()->setContextProperty("sipFixer", sipFixer);
    qDebug() << " uploader";
    viewer->rootContext()->setContextProperty("uploader", uploader);
    qDebug() << " setMainQmlFile";
    viewer->setMainQmlFile(QLatin1String("qrc:/qml/main.qml"));
    qDebug() << " setOrientation";
    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationLockPortrait);
    qDebug() << " showExpanded";
    viewer->showExpanded();

    // Install event filter to capture/release volume keys
    qDebug() << "Install event filters";
    viewer->installEventFilter(mediaKey);

    // Install event filter fixing VKB handing in WebView
    viewer->installEventFilter(sipFixer);

    // De-activate counter if not in the foreground
    // mediaKey->connect(eventFilter, SIGNAL(activate(bool)), counter, SLOT(applicationActivated(bool)));

    // Run application
    qDebug() << "Execute application";
    int ret = app.exec();

    // Delete singletons and exit
    delete viewer;
    delete namFactory;
    Uploader::close();
    SipFixer::close();
    Gft::close();
    Logger::close();
    Platform::close();
    Preferences::close();
    return ret;
}
