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
#include <QProcess>
#include <QUuid>

#if defined(Q_OS_SYMBIAN)
#   include <sysutil.h>
#   include <f32file.h>
#endif // Q_OS_SYMBIAN

#include "platform.h"
#include "preferences.h"

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

/// Minimum free disk space (in K)
const int STEPS_MIN_FREE = 4096;

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
        qWarning() << "Platform.soundUrl: No file for" << name << "in" << (QString(STEPS_DATADIR) + "/sounds");
    }
    return ret;
}

QString Platform::dbPath() {
    QString base(QDir::home().absoluteFilePath(STEPS_BASEDIR));
    return QDir(base).absoluteFilePath("current.db");
}

bool Platform::dbFull() {
    bool ret = false;
    QString base(QDir::home().absoluteFilePath(STEPS_BASEDIR));
#if defined(MEEGO_EDITION_HARMATTAN)
    QProcess df;
    df.start("/bin/df", QStringList() << "-k" << base);
    df.waitForFinished();
    QList<QByteArray> lines = df.readAll().split('\n');
    if (lines.length() > 1) {
        QStringList fields = QString(lines[1]).split(QRegExp("\\s+"));
        if (fields.length() > 3) {
            ret = fields.at(3).toInt() < STEPS_MIN_FREE;
        }
    }
#elif defined(Q_OS_SYMBAN)
    TInt drive = base[0].toLower().unicode() - QChar('a').unicode() + EDriveA;
    ret = SysUtil::DiskSpaceBelowCriticalLevelL(&iFsSession, STEPS_MIN_FREE * 1024, drive);
#endif
    if (ret) {
        qCritical() << "Platform::dbFull: Disk full";
    }
    return ret;
}

QString Platform::deviceId() {
    return Preferences::instance()->value("deviceid", QUuid::createUuid().toString()).toString();
}
