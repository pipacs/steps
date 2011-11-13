#include <QMainWindow>
#include <QKeyEvent>
#include <qplatformdefs.h>

#include "eventfilter.h"

EventFilter::EventFilter(QObject *parent): QObject(parent), active(false) {
#if defined(MEEGO_EDITION_HARMATTAN)
    resourceSet = new ResourcePolicy::ResourceSet("player");
    resourceSet->addResourceObject(new ResourcePolicy::ScaleButtonResource);
#endif
}

EventFilter::~EventFilter() {
#if defined(MEEGO_EDITION_HARMATTAN)
    resourceSet->release();
    resourceSet->deleteResource(ResourcePolicy::ScaleButtonType);
    delete resourceSet;
#endif
}

bool EventFilter::eventFilter(QObject *obj, QEvent *event) {
#if defined(MEEGO_EDITION_HARMATTAN)
    if (event->type() == QEvent::ApplicationDeactivate) {
        active = false;
        resourceSet->release();
    } else if (event->type() == QEvent::ApplicationActivate) {
        active = true;
        resourceSet->acquire();
    } else if (event->type() == QEvent::ActivationChange) {
        qDebug() << "EventFilter::eventFilter: ActivationChange";
        if (active) {
            active = false;
            resourceSet->release();
        } else {
            active = true;
            resourceSet->acquire();
        }
        emit activate(active);
    }
#endif
    return QObject::eventFilter(obj, event);
}
