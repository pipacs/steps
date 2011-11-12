# Add more folders to ship with the application, here
# folder_01.source = qml/steps/meego
# folder_01.target = qml
# DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

symbian:TARGET.UID3 = 0xE1584C4E

# Smart Installer package's UID
# This UID is from the protected range and therefore the package will
# fail to install if self-signed. By default qmake uses the unprotected
# range value if unprotected UID is defined for the application and
# 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=
CONFIG += mobility
MOBILITY += sensors
# MOBILITY += multimedia

# Add dependency to Symbian components
# CONFIG += qt-components

# Maemo Harmattan extras
contains(MEEGO_EDITION,harmattan) {
    CONFIG += link_pkgconfig
    CONFIG += qmsystem2
    PKGCONFIG += libresourceqt1
    RESOURCES += meego.qrc
}

# Symbian extras
symbian {
    RESOURCES += symbian.qrc
    INCLUDEPATH += MW_LAYER_SYSTEMINCLUDE // Not sure if this is needed...
    LIBS += -L\\epoc32\\release\\armv5\\lib -lremconcoreapi
    LIBS += -L\\epoc32\\release\\armv5\\lib -lremconinterfacebase
}

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    counter.cpp \
    ring.cpp \
    eventfilter.cpp \
    preferences.cpp \
    mediakey.cpp

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
    qml/steps/meego/SettingsPage.qml \
    qml/steps/meego/MainPage.qml \
    qml/steps/meego/main.qml \
    qml/steps/meego/Beep.qml \
    qml/steps/symbian/SettingsPage.qml \
    qml/steps/symbian/MainPage.qml \
    qml/steps/symbian/main.qml \
    qml/steps/symbian/Beep.qml \
    sounds/start.wav \
    sounds/beep.wav \
    sounds/stop.wav

HEADERS += \
    counter.h \
    ring.h \
    eventfilter.h \
    preferences.h \
    mediakey.h





