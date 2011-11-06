#include <QtGui/QApplication>
#include <QtDeclarative>

#include "qmlapplicationviewer.h"
#include "counter.h"
#include "eventfilter.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QmlApplicationViewer viewer;
    Counter *counter = new Counter(&viewer);
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.rootContext()->setContextProperty("counter", counter);
    viewer.setMainQmlFile(QLatin1String("qml/steps/main.qml"));
    viewer.showExpanded();

    // Install event filter to capture/release volume keys
    EventFilter *eventFilter = new EventFilter(&viewer);
    viewer.installEventFilter(eventFilter);
    eventFilter->connect(eventFilter, SIGNAL(activate(bool)), counter, SLOT(applicationActivated(bool)));

    int ret = app.exec();
    return ret;
}
