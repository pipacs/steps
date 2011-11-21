#include <QtGui/QApplication>
#include <QtDeclarative>

#include "qmlapplicationviewer.h"
#include "counter.h"
#include "preferences.h"
#include "platform.h"
#include "mediakey.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Show QML viewer
    QmlApplicationViewer viewer;
    Counter *counter = new Counter(&viewer);
    Preferences *prefs = Preferences::instance();
    Platform *platform = Platform::instance();
    Logger *logger = Logger::instance();
    MediaKey *mediaKey = new MediaKey(&viewer);
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.rootContext()->setContextProperty("counter", counter);
    viewer.rootContext()->setContextProperty("prefs", prefs);
    viewer.rootContext()->setContextProperty("platform", platform);
    viewer.rootContext()->setContextProperty("logger", logger);
    viewer.rootContext()->setContextProperty("mediaKey", mediaKey);
    viewer.setMainQmlFile(QLatin1String("qrc:/qml/main.qml"));
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationLockPortrait);
    viewer.showExpanded();

    // Install event filter to capture/release volume keys
    viewer.installEventFilter(mediaKey);
    // mediaKey->connect(eventFilter, SIGNAL(activate(bool)), counter, SLOT(applicationActivated(bool)));

    // Run application
    int ret = app.exec();

    // Delete singletons and exit
    Logger::close();
    Platform::close();
    Preferences::close();
    return ret;
}
