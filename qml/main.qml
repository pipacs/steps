import QtQuick 1.1
import QtMultimediaKit 1.1
import "meego"

StepsPageStackWindow {
    id: main
    initialPage: mainPage
    property int prevCount: 0
    property int prevDailyCount: 0
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

        var delta = count - prevCount
        prevCount = count

        // Register daily step count
        var date = new Date()
        var dateString = date.toDateString()
        if (dateString != prefs.dailyCountDate) {
            resetDailyCount()
            prefs.dailyCountDate = dateString
        }

        var newDailyCount = dailyCount + delta
        if (newDailyCount < 0) {
            newDailyCount = 0
        }

        // Play applause at every X (daily) steps
        var APPLAUSE_GOAL = 10000
        if ((newDailyCount > dailyCount) && ((newDailyCount % APPLAUSE_GOAL) < (dailyCount % APPLAUSE_GOAL))) {
            applause.beep()
        }

        setDailyCount(newDailyCount)
    }

    function resetDailyCount() {
        setDailyCount(0)
        logger.log(counter.count, {"resetDailyCount": 0})
    }

    function resetCount() {
        counter.reset()
        prevCount = 0
        logger.log(0, {"reset": 0})
        resetDailyCount()
    }

    function setDailyCount(d) {
        if (d < 0) {
            d = 0
        }
        dailyCount = d
        prefs.dailyCount = d
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
            prevDailyCount = dailyCount
        }
    }

    Component.onDestruction: {
        logger.log(counter.count, {"appStopped": "com.pipacs.steps"})
    }
}
