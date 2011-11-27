#ifndef SIPFIXER_H
#define SIPFIXER_H

#include <QObject>
#include <QWidget>

// Fix software input panel handling in QML WebView
class SipFixer: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    static SipFixer *instance();
    static void close();
    SipFixer(QObject *parent = 0);
    bool enabled();
    void setEnabled(bool v);
    bool eventFilter(QObject *o, QEvent *e);

signals:
    void enabledChanged();

private:
    QWidget *prevFocusWidget;
    bool enabled_;
};

#endif // SIPFIXER_H
