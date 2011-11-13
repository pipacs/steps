#include <QKeyEvent>

#include "mediakey.h"

#if defined(Q_WS_S60)

// Access Symbian RemCon API
class MediaKeyPrivate: public QObject, public MRemConCoreApiTargetObserver {
public:
    MediaKeyPrivate(MediaKey *parent);
    ~MediaKeyPrivate();
    virtual void MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction aButtonAct);

private:
    CRemConInterfaceSelector *iInterfaceSelector;
    CRemConCoreApiTarget *iCoreTarget;
    MediaKey *d_ptr;
};

MediaKeyPrivate::MediaKeyPrivate(MediaKey *parent): d_ptr(parent) {
    QT_TRAP_THROWING(iInterfaceSelector = CRemConInterfaceSelector::NewL());
    QT_TRAP_THROWING(iCoreTarget = CRemConCoreApiTarget::NewL(*iInterfaceSelector, *this));
    iInterfaceSelector->OpenTargetL();
}

MediaKeyPrivate::~MediaKeyPrivate() {
    delete iInterfaceSelector;
    delete iCoreTarget;
}

// Callback when media keys are pressed
void MediaKeyPrivate::MrccatoCommand(TRemConCoreApiOperationId operation, TRemConCoreApiButtonAction) {
    switch (operation) {
    case ERemConCoreApiVolumeUp:
        emit d_ptr->volumeUpPressed();
        break;
    case ERemConCoreApiVolumeDown:
        emit d_ptr->volumeDownPressed();
        break;
    default:
        break;
    }
}

#endif // Q_WS_S60

MediaKey::MediaKey(QDeclarativeItem *parent): QDeclarativeItem(parent) {
#if defined(MEEGO_EDITION_HARMATTAN)
    resourceSet = new ResourcePolicy::ResourceSet("player");
    resourceSet->addResourceObject(new ResourcePolicy::ScaleButtonResource);
#elif defined(Q_WS_S60)
    d_ptr = new MediaKeyPrivate(this);
#endif
}

MediaKey::~MediaKey() {
#if defined(MEEGO_EDITION_HARMATTAN)
    resourceSet->release();
    resourceSet->deleteResource(ResourcePolicy::ScaleButtonType);
    delete resourceSet;
#endif
}

bool MediaKey::eventFilter(QObject *obj, QEvent *event) {
#if defined(MEEGO_EDITION_HARMATTAN)
    if (event->type() == QEvent::ApplicationDeactivate) {
        active = false;
        resourceSet->release();
    } else if (event->type() == QEvent::ApplicationActivate) {
        active = true;
        resourceSet->acquire();
    } else if (event->type() == QEvent::ActivationChange) {
        qDebug() << "MediaKey::eventFilter: ActivationChange";
        if (active) {
            active = false;
            resourceSet->release();
        } else {
            active = true;
            resourceSet->acquire();
        }
        // emit activate(active);
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_VolumeUp) {
            qDebug() << "MediaKey::eventFilter: Volume up";
            emit volumeUpPressed();
            return true;
        } else if (keyEvent->key() == Qt::Key_VolumeDown) {
            qDebug() << "MediaKey::eventFilter: Vlume down";
            emit volumeDownPressed();
            return true;
        }
    }
#endif
    return QDeclarativeItem::eventFilter(obj, event);
}
