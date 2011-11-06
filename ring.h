#ifndef RING_H
#define RING_H

#include <QtGlobal>
#include <QList>

class Ring {
public:
    explicit Ring(int size, qreal minRise);
    ~Ring();
    void add(qreal measurement);
    void clear();
    int findPeak();
    qreal get(int index);

protected:
    QList<qreal> data;
    int size;
    qreal minRise;
};

#endif // RING_H
