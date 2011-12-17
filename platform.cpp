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

#if defined(Q_OS_SYMBIAN)
#   define STEPS_OS_NAME "symbian"
#elif defined(MEEGO_EDITION_HARMATTAN)
#   define STEPS_OS_NAME "harmattan"
#else
#   define STEPS_OS_NAME "unknown"
#endif

static Platform *theInstance;

Platform::Platform(): QObject() {
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

QString Platform::osName() {
    return STEPS_OS_NAME;
}

QString Platform::appVersion() {
    return QString(STEPS_VERSION);
}

QUrl Platform::soundUrl(const QString &name) {
    QUrl ret;
    QString base = QString(STEPS_DATADIR) + "/sounds/" + name;
    QString wav = base + ".wav";
    QString mp3 = base + ".mp3";
    if (QFileInfo(wav).exists()) {
        ret = QUrl::fromLocalFile(wav);
    } else if (QFileInfo(mp3).exists()) {
        ret = QUrl::fromLocalFile(mp3);
    } else {
        qWarning() << "Platform.soundUrl: No file for" << name;
    }
    qDebug() << "Platform::soundUrl" << name << ":" << ret;
    return ret;
}

QString Platform::dbPath() {
    QString base(QDir::home().absoluteFilePath(STEPS_BASEDIR));
    return QDir(base).absoluteFilePath("current.db");
}
