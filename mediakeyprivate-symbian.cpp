#include "mediakey.h"
#include "mediakeyprivate.h"

#if defined(Q_WS_S60)

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

bool MediaKeyPrivate::eventFilter(QObject *obj, QEvent *event) {
    return QObject::eventFilter(obj, event);
}

#endif // Q_WS_S60

