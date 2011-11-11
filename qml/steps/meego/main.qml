import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: appWindow
    initialPage: mainPage
    focus: true
    showStatusBar: false

    MainPage {
        id: mainPage
    }

    function calibrationChanged(val) {
        console.log("* main.calibrationChanged")
        prefs.calibration = val
    }

    function rawCountChanged(val) {
        console.log("* main.rawCountChanged")
        prefs.rawCount = val
    }

    Component.onCompleted: {
        theme.inverted = true

        counter.calibration = prefs.calibration
        counter.rawCount = prefs.rawCount

        counter.calibrationChanged.connect(appWindow.calibrationChanged)
        counter.rawCountChanged.connect(appWindow.rawCountChanged)
    }
}
