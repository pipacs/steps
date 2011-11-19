VERSION = 0.0.4

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

symbian:TARGET.UID3 = 0xE1584C4E

# Smart Installer package's UID
# This UID is from the protected range and therefore the package will
# fail to install if self-signed. By default qmake uses the unprotected
# range value if unprotected UID is defined for the application and
# 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=
CONFIG += mobility
MOBILITY += sensors
MOBILITY += multimedia

# Add dependency to Symbian components
# CONFIG += qt-components

# Maemo Harmattan
contains(MEEGO_EDITION,harmattan) {
    CONFIG += link_pkgconfig
    CONFIG += qmsystem2
    PKGCONFIG += libresourceqt1
    RESOURCES += meego.qrc
    SOURCES += mediakeyprivate-meego.cpp
    DATADIR = share
    DEFINES += DATADIR=\\\"$$DATADIR\\\"
}

# Symbian
symbian {
    TARGET.CAPABILITY += NetworkServices
    INCLUDEPATH += MW_LAYER_SYSTEMINCLUDE // Not sure if this is needed...
    LIBS += -L\\epoc32\\release\\armv5\\lib -lremconcoreapi
    LIBS += -L\\epoc32\\release\\armv5\\lib -lremconinterfacebase
    SOURCES += mediakeyprivate-symbian.cpp
    RESOURCES += symbian.qrc
    DATADIR = c:/data/steps
    DEFINES += DATADIR='"c:/data/steps"'
}

# Add sounds folder to the application
folder_01.source = sounds
folder_01.target = $$DATADIR
DEPLOYMENTFOLDERS = folder_01

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    counter.cpp \
    ring.cpp \
    preferences.cpp \
    mediakey.cpp \
    platform.cpp

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
    qtc_packaging/version.txt \
    steps256.png \
    qml/ActionsPage.qml \
    qml/meego/StepsButton.qml \
    qml/BigButton.qml \
    qml/symbian/StepsButton.qml

HEADERS += \
    counter.h \
    ring.h \
    preferences.h \
    mediakey.h \
    mediakeyprivate.h \
    platform.h

RESOURCES += \
    common.qrc





