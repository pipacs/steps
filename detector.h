#ifndef DETECTOR_H
#define DETECTOR_H

#include <QObject>

/// Step detector interface.
class Detector: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged)

public:
    explicit Detector(QObject *parent = 0): QObject(parent) {}
    virtual ~Detector() {}
    virtual bool running() = 0;
    virtual int sensitivity() = 0;

signals:
    void step();
    void runningChanged();
    void sensitivityChanged(int value);

public slots:
    virtual void setRunning(bool running) = 0;
    virtual void setSensitivity(int value) = 0;
};

#endif // DETECTOR_H
