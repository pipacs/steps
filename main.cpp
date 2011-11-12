#include <QtGui/QApplication>
#include <QtDeclarative>

#include "qmlapplicationviewer.h"
#include "counter.h"
#include "eventfilter.h"
#include "preferences.h"
#include "mediakey.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

#if defined(Q_WS_S60)
    qmlRegisterType<MediaKey>("MediaKey", 1, 0, "MediaKey");
#endif

    // Show QML viewer
    QmlApplicationViewer viewer;
    Counter *counter = new Counter(&viewer);
    Preferences *prefs = Preferences::instance();
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.rootContext()->setContextProperty("counter", counter);
    viewer.rootContext()->setContextProperty("prefs", prefs);
    viewer.setMainQmlFile(QLatin1String("qrc:/qml/main.qml"));
    viewer.showExpanded();

    // Install event filter to capture/release volume keys
    EventFilter *eventFilter = new EventFilter(0);
    viewer.installEventFilter(eventFilter);
    eventFilter->connect(eventFilter, SIGNAL(activate(bool)), counter, SLOT(applicationActivated(bool)));

    // Run application
    int ret = app.exec();

    // Delete singletons and exit
    delete eventFilter;
    Preferences::close();
    return ret;
}
