import QtQuick 1.1
import QtMultimediaKit 1.1
import "symbian"

StepsPageStackWindow {
    id: main
    initialPage: mainPage
    property int prevCount: 0
    property int dailyCount: 0
    property int activityCount: 0
    property int activity: 0
    property variant activityNames: ["Walking", "Running", prefs.value("activity2Name", "Custom 1"), prefs.value("activity3Name", "Custom 2")]

    MainPage {
        id: mainPage
    }

    Splash {
        id: splash
        Component.onCompleted: splash.activate();
        onFinished: splash.destroy();
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

        // Register activity step count
        setActivityCount(activityCount + delta)

        // Register daily step count
        setDailyCount(dailyCount + delta)
    }

    // Set current activity step count
    function setActivityCount(c) {
        activityCount = c
        if (activityCount < 0) {
            activityCount = 0
        }
        prefs.activityCount = activityCount
    }

    // Set daily step count
    function setDailyCount(c) {
        // Check for new day
        var date = new Date()
        var dateString = date.toDateString()
        if (dateString !== prefs.dailyCountDate) {
            c = 0
            prefs.dailyCountDate = dateString
        }

        dailyCount = c
        if (dailyCount < 0) {
            dailyCount = 0
        }
        prefs.dailyCount = dailyCount
    }

    // Reset current activity counter
    function resetActivityCount() {
        setActivityCount(0)
        logger.log(counter.count, {"resetActivityCount": 0})
    }

    // Reset all counters
    function resetCount() {
        counter.reset()
        prevCount = 0
        logger.log(0, {"reset": 0})
        resetActivityCount()
        setDailyCount(0)
    }

    function runningChanged() {
        logger.log(counter.count, {"counting": counter.running})
    }

    function setActivity(a) {
        if (a !== activity) {
            logger.log(counter.count, {"activity": a})
            activity = a
            prefs.activity = a
            resetActivityCount()
        }
    }

    onActivityNamesChanged: {
        prefs.setValue("activity2Name", activityNames[2])
        prefs.setValue("activity3Name", activityNames[3])
    }

    Component.onCompleted: {
        // counter.calibration = prefs.calibration
        counter.rawCount = prefs.rawCount
        main.prevCount = counter.count
        counter.sensitivity = prefs.sensitivity

        counter.rawCountChanged.connect(main.rawCountChanged)
        counter.step.connect(main.countChanged)
        counter.runningChanged.connect(main.runningChanged)

        logger.log(counter.count, {"appStarted": "com.pipacs.steps", "appVersion": platform.appVersion})

        // Restore step counts from settings
        setDailyCount(prefs.dailyCount)
        setActivity(prefs.activity)
        setActivityCount(prefs.activityCount)
    }

    Component.onDestruction: {
        logger.log(counter.count, {"appStopped": "com.pipacs.steps"})
    }
}
