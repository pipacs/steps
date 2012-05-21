#ifndef MEDIAKEY_H
#define MEDIAKEY_H

#include <QObject>
#include <qplatformdefs.h>

#if defined(MEEGO_EDITION_HARMATTAN)
#   include <policy/resource-set.h>
#else
#   error Unsupported platform
#endif

class MediaKey: public QObject {
    Q_OBJECT

public:
    MediaKey(QObject *parent = 0);
    virtual ~MediaKey();

signals:
    void activate();
    void volumeUpPressed();
    void volumeDownPressed();

public:
    bool eventFilter(QObject *obj, QEvent *event);
    bool active;
    ResourcePolicy::ResourceSet *resourceSet;
};

#endif // MEDIAKEY_H
