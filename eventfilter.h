#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QObject>
#include <policy/resource-set.h>

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
    ResourcePolicy::ResourceSet *resourceSet;
};

#endif // EVENTFILTER_H
