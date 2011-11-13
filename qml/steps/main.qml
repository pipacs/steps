import QtQuick 1.1
import QtMultimediaKit 1.1
import "symbian"
import "meego"

MainWindow {
    id: appWindow
    initialPage: mainPage

    MainPage {
        id: mainPage
    }

    Beep {
        id: stepSound
        source: "/sounds/beep.wav"
    }

    function rawCountChanged(val) {
        prefs.rawCount = val
    }

    function countChanged() {
        if (counter.count) {
            stepSound.play()
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
