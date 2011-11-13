#include "mediakey.h"
#include "mediakeyprivate.h"

MediaKey::MediaKey(QObject *parent): QObject(parent) {
    d_ptr = new MediaKeyPrivate(this);
}

bool MediaKey::eventFilter(QObject *obj, QEvent *event) {
    return d_ptr->eventFilter(obj, event);
}
