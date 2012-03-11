#include <QApplication>
#include <QInputContext>
#include <QDebug>
#include <qplatformdefs.h>

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
#if defined(MEEGO_EDITION_HARMATTAN)
    if (enabled_) {
        QInputContext *ic = qApp->inputContext();
        if (ic) {
            QWidget *focusWidget = ic->focusWidget();
            if (!focusWidget && prevFocusWidget) {
                qDebug() << "SipFixer::eventFilter: Close SIP";
                QEvent closeSIPEvent(QEvent::CloseSoftwareInputPanel);
                ic->filterEvent(&closeSIPEvent);
            } else if (!prevFocusWidget && focusWidget) {
                qDebug() << "SipFixer::eventFilter: Open SIP";
                QEvent openSIPEvent(QEvent::RequestSoftwareInputPanel);
                ic->filterEvent(&openSIPEvent);
            }
            prevFocusWidget = focusWidget;
        }
    }
#endif
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
