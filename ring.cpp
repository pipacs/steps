#include <assert.h>
#include <string.h>
#include <QDebug>

#include "ring.h"

Ring::Ring(int size_, qreal minRise_) {
    size = size_;
    data = new qreal[size];
    memset(data, 0, size * sizeof(qreal));
    head = 0;
    total = 0;
    setMinimumRise(minRise_);
}

Ring::~Ring() {
    qDebug() << "Ring::~Ring";
    delete data;
}

void Ring::add(qreal measurement) {
    data[head] = measurement;
    head = (head + 1) % size;
    if (++total > size) {
        total = size;
    }
}

void Ring::clear() {
    if (total) {
        qreal last = get(total - 1);
        memset(data, 0, size * sizeof(qreal));
        head = 0;
        total = 0;
        add(last);
    }
}

int Ring::findPeak() {
    qreal travel = 0;
    if (total == 0) {
        return -1;
    }
    int count = total - 1;
    qreal data0 = get(0);
    for (int i = 0; i < count; i++) {
        qreal dataI = get(i);
        travel += get(i + 1) - dataI;
        qreal rise = dataI - data0 + minRise;
        if ((travel / rise) > 1.f) {
            return i;
        }
    }
    return -1;
}

qreal Ring::get(int index) {
    assert(index < total);
    return data[(head + index) % total];
}

void Ring::setMinimumRise(qreal minRise_) {
    minRise = minRise_;
    if (minRise < 0.1) {
        minRise = 0.1;
    }
}
