#include <assert.h>
#include "ring.h"

Ring::Ring(int size_, qreal minRise_) {
    size = size_;
    setMinRise(minRise_);
}

Ring::~Ring() {
}

void Ring::add(qreal measurement) {
    if (data.count() == size) {
        data.removeFirst();
    }
    data.append(measurement);
}

void Ring::clear() {
    if (data.count()) {
        qreal last = data[data.count() - 1];
        data.clear();
        add(last);
    }
}

int Ring::findPeak() {
    qreal travel = 0;
    int count = data.count() - 1;
    assert(count >= 0);
    for (int i = 0; i < count; i++) {
        travel += data[i + 1] - data[i];
        qreal rise = data[i] - data[0] + minRise;
        if ((travel / rise) > 1.f) {
            return i;
        }
    }
    return -1;
}

qreal Ring::get(int index) {
    return data[index];
}

void Ring::setMinRise(qreal minRise_) {
    minRise = minRise_;
    if (minRise < 0.1) {
        minRise = 0.1;
    }
}
