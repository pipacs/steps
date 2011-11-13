#ifndef MEDIAKEY_H
#define MEDIAKEY_H

#include <QDeclarativeItem>
#include <qplatformdefs.h>

#if defined(MEEGO_EDITION_HARMATTAN)
#   include <policy/resource-set.h>
#elif defined(Q_WS_S60)
#   include <remconcoreapitargetobserver.h>
#   include <remconcoreapitarget.h>
#   include <remconinterfaceselector.h>
    class MediaKeyPrivate;
#endif

class MediaKey: public QDeclarativeItem {
    Q_OBJECT
public:
    MediaKey(QDeclarativeItem *parent = 0);
    ~MediaKey();

signals:
    void volumeDownPressed();
    void volumeUpPressed();
    void activate();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
#if defined(MEEGO_EDITION_HARMATTAN)
    bool active;
    ResourcePolicy::ResourceSet *resourceSet;
#elif defined(Q_WS_S60)
    MediaKeyPrivate *d_ptr;
    friend class MediaKeyPrivate;
#endif
};

#endif // MEDIAKEY_H
