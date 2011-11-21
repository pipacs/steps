#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>
#include <QDebug>
#include <qplatformdefs.h>
#include <QDir>

#include "platform.h"

#if defined(Q_OS_SYMBIAN)
#   define STEPS_BASEDIR "steps"
#elif defined(MEEGO_EDITION_HARMATTAN)
#   define STEPS_BASEDIR ".steps"
#else
#   error Unknown platform
#endif

static Platform *theInstance;

Platform::Platform(): QObject() {
}

Platform::~Platform() {
    qDebug() << "Platform::~Platform";
}

Platform *Platform::instance() {
    if (!theInstance) {
        theInstance = new Platform();
    }
    return theInstance;
}

void Platform::close() {
    delete theInstance;
    theInstance = 0;
}

QString Platform::version() {
    return QString(STEPS_VERSION);
}

void Platform::browse(const QString &url) {
    QDesktopServices::openUrl(QUrl(url));
}

QUrl Platform::soundUrl(const QString &name) {
    QUrl ret = QUrl::fromLocalFile(QString(STEPS_DATADIR) + "/sounds/" + name + ".wav");
    qDebug() << "Platform::soundUrl" << name << ":" << ret;
    return ret;
}

QString Platform::dbPath() {
    QString base(QDir::home().absoluteFilePath(STEPS_BASEDIR));
    return QDir(base).absoluteFilePath("log.db");
}
