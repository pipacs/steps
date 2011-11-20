import QtQuick 1.1

StepsPageStackWindow {
    id: appWindow
    initialPage: mainPage

    MainPage {
        id: mainPage
    }

    function rawCountChanged(val) {
        prefs.rawCount = val
    }

    function countChanged() {
        if (counter.count) {
            // stepSound.play()
        }
    }

    Component.onCompleted: {
        counter.calibration = prefs.calibration
        counter.rawCount = prefs.rawCount
        counter.sensitivity = prefs.sensitivity

        counter.rawCountChanged.connect(appWindow.rawCountChanged)
        counter.step.connect(appWindow.countChanged)
    }
}
