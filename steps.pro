VERSION = 0.0.6

# Qt packages to use
CONFIG += mobility
MOBILITY += sensors
MOBILITY += multimedia
QT += sql
QT += network

# Add dependency to Symbian components
# CONFIG += qt-components

DEFINES += KQOAUTH

# Platform-specific
contains(MEEGO_EDITION,harmattan) {
    CONFIG += link_pkgconfig
    CONFIG += qmsystem2
    PKGCONFIG += libresourceqt1
    RESOURCES += meego.qrc
    SOURCES += mediakeyprivate-meego.cpp
    DATADIR = share
    DEFINES += STEPS_DATADIR=\\\"/opt/steps/share\\\"
    DEFINES += STEPS_VERSION=\\\"$$VERSION\\\"
    QML_IMPORT_PATH = qml/meego
} else:symbian {
    TARGET.CAPABILITY += NetworkServices
    TARGET.UID3 = 0xE1584C4E
    INCLUDEPATH += MW_LAYER_SYSTEMINCLUDE // Not sure if this is needed...
    LIBS += -L\\epoc32\\release\\armv5\\lib -lremconcoreapi
    LIBS += -L\\epoc32\\release\\armv5\\lib -lremconinterfacebase
    SOURCES += mediakeyprivate-symbian.cpp
    RESOURCES += symbian.qrc
    DATADIR = c:/data/steps
    DEFINES += STEPS_DATADIR='"c:/data/steps"'
    DEFINES += STEPS_VERSION='"$$VERSION"'
    QML_IMPORT_PATH = qml/symbian
    # Smart Installer package's UID
    # This UID is from the protected range and therefore the package will
    # fail to install if self-signed. By default qmake uses the unprotected
    # range value if unprotected UID is defined for the application and
    # 0x2002CCCF value if protected UID is given to the application
    #symbian:DEPLOYMENT.installer_header = 0x2002CCCF
} else {
    error("Unsupported platform")
}

# Add sounds folder to the application.
# Sounds cannot be played back from resource on Symbian
folder_01.source = sounds
folder_01.target = $$DATADIR
DEPLOYMENTFOLDERS = folder_01

SOURCES += \
    main.cpp \
    counter.cpp \
    ring.cpp \
    preferences.cpp \
    mediakey.cpp \
    platform.cpp \
    logger.cpp \
    sipfixer.cpp \
    kqoauth/kqoauthauthreplyserver.cpp \
    kqoauth/kqoauthmanager.cpp \
    kqoauth/kqoauthrequest.cpp \
    kqoauth/kqoauthrequest_1.cpp \
    kqoauth/kqoauthrequest_xauth.cpp \
    kqoauth/kqoauthutils.cpp \
    uploader.cpp \
    gftprogram.cpp \
    gft.cpp

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    README.txt \
    qml/symbian/Beep.qml \
    sounds/start.wav \
    sounds/beep.wav \
    sounds/stop.wav \
    background.svg \
    qml/Beep.qml \
    qml/main.qml \
    qml/Dummy.qml \
    sounds/settings.wav \
    qml/MainPage.qml \
    qml/meego/StepsLabel.qml \
    qml/meego/StepsPage.qml \
    qml/meego/StepsPageStackWindow.qml \
    qml/meego/StepsCheckBox.qml \
    qml/meego/StepsSlider.qml \
    qml/meego/StepsDialog.qml \
    qml/SettingsPage.qml \
    qml/symbian/StepsPageStackWindow.qml \
    qml/symbian/StepsPage.qml \
    qml/symbian/StepsLabel.qml \
    qml/symbian/StepsCheckBox.qml \
    qml/symbian/StepsSlider.qml \
    qml/symbian/StepsDialog.qml \
    steps256.png \
    qml/ActionsPage.qml \
    qml/meego/StepsButton.qml \
    qml/BigButton.qml \
    qml/symbian/StepsButton.qml \
    qml/symbian/StepsYesNoDialog.qml \
    images/steps.png \
    qml/meego/StepsYesNoDialog.qml \
    images/settings.png \
    images/reset.png \
    qml/meego/StepsTextField.qml \
    qml/LoginBrowser.qml \
    qml/meego/StepsSpinner.qml \
    qml/symbian/StepsSpinner.qml

HEADERS += \
    counter.h \
    ring.h \
    preferences.h \
    mediakey.h \
    mediakeyprivate.h \
    platform.h \
    logger.h \
    sipfixer.h \
    kqoauth/kqoauthauthreplyserver.h \
    kqoauth/kqoauthauthreplyserver_p.h \
    kqoauth/kqoauthglobals.h \
    kqoauth/kqoauthmanager.h \
    kqoauth/kqoauthmanager_p.h \
    kqoauth/kqoauthrequest.h \
    kqoauth/kqoauthrequest_1.h \
    kqoauth/kqoauthrequest_p.h \
    kqoauth/kqoauthrequest_xauth.h \
    kqoauth/kqoauthrequest_xauth_p.h \
    kqoauth/kqoauthutils.h \
    uploader.h \
    gftprogram.h \
    gft.h

RESOURCES += \
    common.qrc








