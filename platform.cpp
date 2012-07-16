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
#include <QDesktopServices>
#include <QLocale>
#include <QDateTime>

#if defined(Q_OS_SYMBIAN)
#   include <sysutil.h>
#   include <f32file.h>
#   include <psmclient.h>
#   include <psmsettings.h>
#elif defined(MEEGO_EDITION_HARMATTAN)
#   include <qmdevicemode.h>
#   include <qmsysteminformation.h>
#endif

#include "platform.h"
#include "preferences.h"
#include "trace.h"

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

#if defined(Q_OS_SYMBIAN)
class PsmObserver: public MPsmClientObserver {
public:
    void PowerSaveModeChangeError(const TInt error) {
        qDebug() << "PsmObserver::PowerSaveModeChangeError" << error;
    }

    void PowerSaveModeChanged(const TPsmsrvMode mode) {
        qDebug() << "PsmObserver::PowerSaveModeChanged: To" << (int)mode;
    }
};
#endif

Platform::Platform(): QObject() {
#if defined(Q_OS_SYMBIAN)
    psmObserver = new PsmObserver;
    QT_TRAP_THROWING(psmClient = CPsmClient::NewL(*psmObserver));
#endif
}

Platform::~Platform() {
#if defined(Q_OS_SYMBIAN)
    delete psmClient;
    delete psmObserver;
#endif
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

QString Platform::osVersion() {
#if defined(Q_OS_SYMBIAN)
    return "3";
#elif defined(MEEGO_EDITION_HARMATTAN)
    QString firmwareVersion = MeeGo::QmSystemInformation().valueForKey("/device/sw-release-ver");
    if (!firmwareVersion.startsWith("DFL61_HARMATTAN_") || firmwareVersion.length() < 20) {
        return "unknown";
    }
    int majorVersion = firmwareVersion.at(16).digitValue();
    int minorVersion = firmwareVersion.at(17).digitValue();
    if (majorVersion >= 3) {
        return "1.2";
    } else if (majorVersion == 2 && minorVersion >= 2) {
        return "1.1.1";
    } else if (majorVersion == 2) {
        return "1.1";
    } else if (majorVersion == 1) {
        return "1.0";
    } else {
        return "unknown";
    }
#else
    return QString();
#endif
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
    return QDir(base).absoluteFilePath("current.dc");
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
#elif defined(Q_OS_SYMBIAN)
    TInt drive = base[0].toLower().unicode() - QChar('a').unicode() + EDriveA;
    ret = SysUtil::DiskSpaceBelowCriticalLevelL(0, STEPS_MIN_FREE * 1024, drive);
#endif
    if (ret) {
        qCritical() << "Platform::dbFull: Disk full";
    }
    return ret;
}

QString Platform::deviceId() {
    QString id = Preferences::instance()->value("deviceid").toString();
    if (!id.length()) {
        id = QUuid::createUuid().toString();
        Preferences::instance()->setValue("deviceid", id);
    }
    return id;
}

void Platform::setSavePower(bool v) {
#if defined(MEEGO_EDITION_HARMATTAN)
    if (MeeGo::QmDeviceMode().setPSMState(v? MeeGo::QmDeviceMode::PSMStateOn: MeeGo::QmDeviceMode::PSMStateOff)) {
        emit savePowerChanged();
    }
#elif defined(Q_OS_SYMBIAN)
    psmClient->ChangePowerSaveMode(v? EPsmsrvModePowerSave: EPsmsrvModeNormal);
    emit savePowerChanged();
#endif
}

bool Platform::savePower() {
#if defined(MEEGO_EDITION_HARMATTAN)
    return MeeGo::QmDeviceMode().getPSMState() == MeeGo::QmDeviceMode::PSMStateOn;
#elif defined(Q_OS_SYMBIAN)
    TInt mode;
    (void)psmClient->PsmSettings().GetCurrentMode(mode);
    return mode == EPsmsrvModePowerSave;
#else
    return false;
#endif
}

QString Platform::text(const QString &key) {
    QString locale = QLocale::system().name();
    QFile resource(":/texts/" + locale + "/" + key);
    if (!resource.open(QFile::ReadOnly)) {
        resource.setFileName(":/texts/" + locale.left(2) + "/" + key);
        if (!resource.open(QFile::ReadOnly)) {
            resource.setFileName(":/texts/" + key);
            (void)resource.open(QFile::ReadOnly);
        }
    }
    QString ret = QString::fromUtf8(resource.readAll().constData());
    resource.close();
    return ret;
}

QString Platform::traceFileName() const {
#if defined(Q_OS_SYMBIAN)
    return "e:/data/steps.log";
#else
    return QDir(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)).absoluteFilePath("steps.log");
#endif
}

void Platform::traceToFile(bool enable) {
    Trace::setFileName(enable? traceFileName(): "");
}

void Platform::deleteTraceFile() {
    Trace::setFileName("");
    QFile(traceFileName()).remove();
}

qint64 Platform::time() {
    return QDateTime::currentMSecsSinceEpoch() / 1000;
}
