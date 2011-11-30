import QtQuick 1.1
import QtMultimediaKit 1.1
import "meego"

StepsPageStackWindow {
    id: appWindow
    initialPage: mainPage
    property int prevCount: counter.count

    MainPage {
        id: mainPage
    }

    Beep {
        id: applause
        source: platform.soundUrl("applause")
    }

    function rawCountChanged(val) {
        prefs.rawCount = val
    }

    function countChanged(count) {
        if (count === 0) {
            return
        }

        logger.log(count, {})
        if ((count % 10000) < (prevCount % 10000)) {
            applause.play()
        }
        prevCount = count
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
