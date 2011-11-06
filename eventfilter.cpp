#include <QMainWindow>
#include <QKeyEvent>

#include "eventfilter.h"

EventFilter::EventFilter(QObject *parent): QObject(parent), active(false) {
    resourceSet = new ResourcePolicy::ResourceSet("player");
    resourceSet->addResourceObject(new ResourcePolicy::ScaleButtonResource);
}

EventFilter::~EventFilter() {
    resourceSet->release();
    resourceSet->deleteResource(ResourcePolicy::ScaleButtonType);
    delete resourceSet;
}

bool EventFilter::eventFilter(QObject *obj, QEvent *event) {
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
    return QObject::eventFilter(obj, event);
}
