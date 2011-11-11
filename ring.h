#ifndef RING_H
#define RING_H

#include <QtGlobal>
#include <QList>

/// Primitive ring buffer that can detect peaks.
class Ring {
public:
    explicit Ring(int size, qreal minRise);
    ~Ring();
    void add(qreal measurement);
    void clear();
    int findPeak();
    qreal get(int index);
    void setMinRise(qreal value);

protected:
    QList<qreal> data;
    int size;
    qreal minRise;
};

#endif // RING_H
