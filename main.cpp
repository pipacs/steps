#include <QtGui/QApplication>
#include <QtDeclarative>

#include "qmlapplicationviewer.h"
#include "counter.h"
#include "preferences.h"
#include "mediakey.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qmlRegisterType<MediaKey>("MediaKey", 1, 0, "MediaKey");

    // Show QML viewer
    QmlApplicationViewer viewer;
    Counter *counter = new Counter(&viewer);
    Preferences *prefs = Preferences::instance();
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.rootContext()->setContextProperty("counter", counter);
    viewer.rootContext()->setContextProperty("prefs", prefs);
    viewer.setMainQmlFile(QLatin1String("qrc:/qml/main.qml"));
    viewer.showExpanded();

#if defined(MEEGO_EDITION_HARMATTAN)
    // Install event filter to capture/release volume keys
    MediaKey *eventFilter = new MediaKey(0);
    viewer.installEventFilter(eventFilter);
    // eventFilter->connect(eventFilter, SIGNAL(activate(bool)), counter, SLOT(applicationActivated(bool)));
#endif

    // Run application
    int ret = app.exec();

    // Delete singletons and exit
#if defined(MEEGO_EDITION_HARMATTAN)
    delete eventFilter;
#endif
    Preferences::close();
    return ret;
}
