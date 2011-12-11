import QtQuick 1.1
import QtMultimediaKit 1.1
import "meego"

StepsPageStackWindow {
    id: main
    initialPage: mainPage
    property int prevCount: 0
    property int dailyCount: 0

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

        // Play applause at every X steps
        var APPLAUSE_GOAL = 10000
        if ((count > prevCount) && ((count % APPLAUSE_GOAL) < (prevCount % APPLAUSE_GOAL))) {
            applause.beep()
        }

        var delta = count - prevCount
        prevCount = count

        // Register daily step count
        var date = new Date()
        var dateString = date.toDateString()
        if (dateString != prefs.dailyCountDate) {
            resetDailyCount()
            prefs.dailyCountDate = dateString
        }
        dailyCount += delta
        if (dailyCount < 0) {
            dailyCount = 0
        }
        prefs.dailyCount = dailyCount
    }

    function resetDailyCount() {
        dailyCount = 0
        prefs.dailyCount = 0
        logger.log(counter.count, {"resetDailyCount": 0})
    }

    function resetCount() {
        counter.reset()
        prevCount = 0
        logger.log(0, {"reset": 0})
        resetDailyCount()
    }

    function runningChanged() {
        logger.log(counter.count, {"counting": counter.running})
    }

    Component.onCompleted: {
        counter.calibration = prefs.calibration
        counter.rawCount = prefs.rawCount
        main.prevCount = counter.count
        counter.sensitivity = prefs.sensitivity

        counter.rawCountChanged.connect(main.rawCountChanged)
        counter.step.connect(main.countChanged)
        counter.runningChanged.connect(main.runningChanged)

        logger.log(counter.count, {"appStarted": "com.pipacs.steps", "appVersion": platform.version})

        // Restore daily step count from settings
        var date = new Date()
        var dateString = date.toDateString()
        if (dateString != prefs.dailyCountDate) {
            resetDailyCount()
            prefs.dailyCountDate = dateString
        } else {
            dailyCount = prefs.dailyCount
        }
    }

    Component.onDestruction: {
        logger.log(counter.count, {"appStopped": "com.pipacs.steps"})
    }
}
