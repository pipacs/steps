#ifndef MEDIAKEY_H
#define MEDIAKEY_H

#include <QDeclarativeItem>

#ifdef Q_WS_S60

#include <remconcoreapitargetobserver.h>
#include <remconcoreapitarget.h>
#include <remconinterfaceselector.h>

class MediaKeyPrivate;
class MediaKey: public QDeclarativeItem {
    Q_OBJECT
public:
    MediaKey(QDeclarativeItem *parent = 0);
    // void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:
    void volumeDownPressed();
    void volumeUpPressed();

private:
    MediaKeyPrivate *d_ptr;
    friend class MediaKeyPrivate;
};

#endif // Q_WS_S60
#endif // MEDIAKEY_H
