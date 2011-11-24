#include <QtGui/QApplication>
#include <QtDeclarative>

#include "qmlapplicationviewer.h"
#include "counter.h"
#include "preferences.h"
#include "platform.h"
#include "mediakey.h"
#include "logger.h"
#include "googledocs.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Show QML viewer
    Preferences *prefs = Preferences::instance();
    Platform *platform = Platform::instance();
    Logger *logger = Logger::instance();
    GoogleDocs *googleDocs = GoogleDocs::instance();
    QmlApplicationViewer *viewer = new QmlApplicationViewer;
    Counter *counter = new Counter(viewer);
    MediaKey *mediaKey = new MediaKey(viewer);
    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer->rootContext()->setContextProperty("counter", counter);
    viewer->rootContext()->setContextProperty("prefs", prefs);
    viewer->rootContext()->setContextProperty("platform", platform);
    viewer->rootContext()->setContextProperty("logger", logger);
    viewer->rootContext()->setContextProperty("mediaKey", mediaKey);
    viewer->rootContext()->setContextProperty("googleDocs", googleDocs);
    viewer->setMainQmlFile(QLatin1String("qrc:/qml/main.qml"));
    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationLockPortrait);
    viewer->showExpanded();

    // Install event filter to capture/release volume keys
    viewer->installEventFilter(mediaKey);

    // De-activate counter if not in the foreground
    // mediaKey->connect(eventFilter, SIGNAL(activate(bool)), counter, SLOT(applicationActivated(bool)));

    // Run application
    int ret = app.exec();

    // Delete singletons and exit
    delete viewer;
    GoogleDocs::close();
    Logger::close();
    Platform::close();
    Preferences::close();
    return ret;
}
