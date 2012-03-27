import QtQuick 1.1
import QtMultimediaKit 1.1
import "meego"

StepsPageStackWindow {
    id: main
    initialPage: mainPage
    property int totalCount: 0
    property int dailyCount: 0
    property int activityCount: 0
    property int activity: 0
    property variant activityNames: [qsTr("Walking"), qsTr("Running"), prefs.value("activity2Name", qsTr("Custom 1")), prefs.value("activity3Name", qsTr("Custom 2"))]

    MainPage {
        id: mainPage
    }

    Splash {
        id: splash
        Component.onCompleted: splash.activate();
        onFinished: splash.destroy();
    }

    function onStepDetected() {
        // Register total step count
        logger.log(1, {})
        totalCount += 1
        prefs.rawCount = totalCount

        // Register activity step count
        setActivityCount(activityCount + 1)

        // Register daily step count
        setDailyCount(dailyCount + 1)
    }

    // Set current activity step count
    function setActivityCount(c) {
        activityCount = c
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
        prefs.dailyCount = dailyCount
    }

    // Reset current activity counter
    function resetActivityCount() {
        setActivityCount(0)
        logger.log(0, {"resetActivityCount": 0})
    }

    // Reset all counters
    function resetCount() {
        detector.reset()
        logger.log(0, {"reset": 0})
        resetActivityCount()
        setDailyCount(0)
    }

    function runningChanged() {
        logger.log(0, {"counting": detector.running})
        if (prefs.savePower) {
            platform.savePower = detector.running
        }
    }

    function setActivity(a) {
        if (a !== activity) {
            logger.log(0, {"activity": a})
            activity = a
            prefs.activity = a
            resetActivityCount()
        }
    }

    function onDetectedActivityChanged() {
        logger.log(0, {"detectedActivity": detector.activity})
    }

    onActivityNamesChanged: {
        prefs.setValue("activity2Name", activityNames[2])
        prefs.setValue("activity3Name", activityNames[3])
    }

    Component.onCompleted: {
        detector.sensitivity = prefs.sensitivity
        detector.step.connect(main.onStepDetected)
        detector.runningChanged.connect(main.runningChanged)
        detector.activityChanged.connect(main.onDetectedActivityChanged)
        logger.log(0, {"appStarted": "com.pipacs.steps", "appVersion": platform.appVersion, "osName": platform.osName})

        // Restore step counts from settings
        totalCount = prefs.rawCount
        setDailyCount(prefs.dailyCount)
        setActivity(prefs.activity)
        setActivityCount(prefs.activityCount)
    }

    Component.onDestruction: {
        if (prefs.savePower) {
            platform.savePower = false
        }
        logger.log(0, {"appStopped": "com.pipacs.steps"})
    }
}
