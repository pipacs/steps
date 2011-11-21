import QtQuick 1.1
import "meego"

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
            logger.log(counter.count, {})
        }
    }

    function runningChanged() {
        logger.log(counter.count, {"counting": counter.running})
    }

    Component.onCompleted: {
        counter.calibration = prefs.calibration
        counter.rawCount = prefs.rawCount
        counter.sensitivity = prefs.sensitivity

        counter.rawCountChanged.connect(appWindow.rawCountChanged)
        counter.step.connect(appWindow.countChanged)
        counter.runningChanged.connect(appWindow.runningChanged)

        logger.log(counter.count, {"appStarted": "com.pipacs.steps", "appVersion": platform.version})
    }

    Component.onDestruction: {
        logger.log(counter.count, {"appStopped": "com.pipacs.steps"})
    }
}
