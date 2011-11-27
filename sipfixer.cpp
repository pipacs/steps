#include <QApplication>
#include <QInputContext>

#include "sipfixer.h"

static SipFixer *instance_;

SipFixer *SipFixer::instance() {
    if (!instance_) {
        instance_ = new SipFixer();
    }
    return instance_;
}

void SipFixer::close() {
    delete instance_;
    instance_ = 0;
}

SipFixer::SipFixer(QObject *parent): QObject(parent), prevFocusWidget(0), enabled_(false) {
}

bool SipFixer::eventFilter(QObject *obj, QEvent *event) {
    QInputContext *ic = qApp->inputContext();
    if (ic) {
        if (enabled_) {
            if (ic->focusWidget() == 0 && prevFocusWidget) {
                QEvent closeSIPEvent(QEvent::CloseSoftwareInputPanel);
                ic->filterEvent(&closeSIPEvent);
            } else if (prevFocusWidget == 0 && ic->focusWidget()) {
                QEvent openSIPEvent(QEvent::RequestSoftwareInputPanel);
                ic->filterEvent(&openSIPEvent);
            }
        }
        prevFocusWidget = ic->focusWidget();
    }
    return QObject::eventFilter(obj, event);
}

bool SipFixer::enabled() {
    return enabled_;
}

void SipFixer::setEnabled(bool v) {
    enabled_ = v;
    if (!enabled_) {
        QInputContext *ic = qApp->inputContext();
        if (ic) {
            QEvent closeSipEvent(QEvent::CloseSoftwareInputPanel);
            ic->filterEvent(&closeSipEvent);
        }
    }
    emit enabledChanged();
}
