#include "mediakey.h"

#ifdef Q_WS_S60

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

// Constructor
MediaKey::MediaKey(QDeclarativeItem *parent): QDeclarativeItem(parent) {
    d_ptr = new MediaKeyPrivate(this);
}

// The paint method
// void MediaKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
// {
    // This item has no visual
// }

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
