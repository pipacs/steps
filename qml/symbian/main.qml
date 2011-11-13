import QtQuick 1.1
import com.nokia.symbian 1.1
import QtMultimediaKit 1.1

PageStackWindow {
    id: appWindow
    initialPage: mainPage
    focus: true
    // showStatusBar: false

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
        // theme.inverted = true

        counter.calibration = prefs.calibration
        counter.rawCount = prefs.rawCount
        counter.sensitivity = prefs.sensitivity

        counter.rawCountChanged.connect(appWindow.rawCountChanged)
        counter.step.connect(appWindow.countChanged)
    }
}
