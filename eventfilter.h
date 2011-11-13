#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QObject>
#include <qplatformdefs.h>

#if defined(MEEGO_EDITION_HARMATTAN)
#include <policy/resource-set.h>
#endif

/// Acquire/release volume keys on activation change.
class EventFilter: public QObject {
    Q_OBJECT

public:
    explicit EventFilter(QObject *parent = 0);
    ~EventFilter();

signals:
    void activate(bool active);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    bool active;
#if defined(MEEGO_EDITION_HARMATTAN)
    ResourcePolicy::ResourceSet *resourceSet;
#endif
};

#endif // EVENTFILTER_H
