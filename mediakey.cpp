#include <QDebug>
#include <QKeyEvent>

#include "mediakey.h"

MediaKey::MediaKey(QObject *parent): QObject(parent) {
    resourceSet = new ResourcePolicy::ResourceSet("player");
    resourceSet->addResourceObject(new ResourcePolicy::ScaleButtonResource);
}

MediaKey::~MediaKey() {
    resourceSet->release();
    resourceSet->deleteResource(ResourcePolicy::ScaleButtonType);
    delete resourceSet;
}

bool MediaKey::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::ApplicationDeactivate) {
        active = false;
        resourceSet->release();
    } else if (event->type() == QEvent::ApplicationActivate) {
        active = true;
        resourceSet->acquire();
    } else if (event->type() == QEvent::ActivationChange) {
        if (active) {
            active = false;
            resourceSet->release();
        } else {
            active = true;
            resourceSet->acquire();
        }
        // emit d_ptr->activate(active);
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_VolumeUp) {
            emit volumeUpPressed();
            return true;
        } else if (keyEvent->key() == Qt::Key_VolumeDown) {
            emit volumeDownPressed();
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}
