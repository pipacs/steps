import QtQuick 1.1
import com.nokia.meego 1.0
import QtMultimediaKit 1.1

PageStackWindow {
    id: appWindow
    initialPage: mainPage
    focus: true
    showStatusBar: false

    MainPage {
        id: mainPage
    }

    SoundEffect {
        id: stepSound
        source: "file:///usr/share/sounds/ui-tones/snd_camera_shutter.wav"
    }

    function calibrationChanged(val) {
        console.log("* main.calibrationChanged")
        prefs.calibration = val
    }

    function rawCountChanged(val) {
        console.log("* main.rawCountChanged")
        prefs.rawCount = val
    }

    function step() {
        // stepSound.play()
    }

    Component.onCompleted: {
        theme.inverted = true

        counter.calibration = prefs.calibration
        counter.rawCount = prefs.rawCount

        counter.calibrationChanged.connect(appWindow.calibrationChanged)
        counter.rawCountChanged.connect(appWindow.rawCountChanged)
        counter.step.connect(appWindow.step)
    }
}
