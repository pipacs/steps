import QtQuick 1.1
import QtMultimediaKit 1.1
import "symbian"

StepsPageStackWindow {
    id: main
    initialPage: mainPage
    property int totalCount: 0
    property int dailyCount: 0
    property int activityCount: 0
    property int lastActivity: 0
    property int unsavedCount: 0
    property int lastSaveTime: 0

    MainPage {
        id: mainPage
    }

    SettingsPage {
        id: settingsPage
    }

    AboutPage {
        id: aboutPage
    }

    Splash {
        id: splash
        Component.onCompleted: splash.activate();
        onFinished: splash.destroy();
    }

    Beep {
        id: walkingSound
        source: platform.soundUrl("walking")
    }

    Beep {
        id: runningSound
        source: platform.soundUrl("running")
    }

    Beep {
        id: idleSound
        source: platform.soundUrl("idle")
    }

    Beep {
        id: stepSound
        source: platform.soundUrl("beep")
    }

    StepsBanner {
        id: linkInfo
        text: gft.linked? qsTr("Logged in to Google Docs"): qsTr("Logged out from Google Docs")
    }

    function onStepDetected(count) {
        // Accumulate unsaved step count
        unsavedCount += count
        totalCount += count
        prefs.rawCount = totalCount

        // Log step count to database
        var now = platform.time()
        if ((unsavedCount > 50) || ((now - lastSaveTime) > 300)) {
            console.log("* main.onStepDetected: Logging " + unsavedCount + " steps")
            logger.log(unsavedCount, {})
            unsavedCount = 0
            lastSaveTime = now
        }

        // Register activity step count
        setActivityCount(activityCount + count)

        // Register daily step count
        setDailyCount(dailyCount + count)
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
    }

    // Reset all counters
    function resetCount() {
        detector.reset()
        resetActivityCount()
        setDailyCount(0)
    }

    function runningChanged() {
        if (prefs.savePower) {
            platform.savePower = detector.running
        }
    }

    function onDetectedActivityChanged() {
        if ((detector.activity > 1) && (detector.activity !== lastActivity)) {
            lastActivity = detector.activity
            logger.log(0, {"detectedActivity": detector.activity})
        }
        if (detector.activity === 2) {
            walkingSound.beep()
        } else if (detector.activity === 3) {
            runningSound.beep()
        } else {
            idleSound.beep()
        }
    }

    Component.onCompleted: {
        detector.runningStepTimeDiff = prefs.runningStepTimeDiff
        detector.minReadingDiff = prefs.minReadingDiff
        detector.step.connect(main.onStepDetected)
        detector.runningChanged.connect(main.runningChanged)
        detector.activityChanged.connect(main.onDetectedActivityChanged)
        logger.log(0, {"appStarted": "com.pipacs.steps", "appVersion": platform.appVersion, "osName": platform.osName, "activity": prefs.activity, "counting": false})

        // Restore step counts from settings
        totalCount = prefs.rawCount
        setDailyCount(prefs.dailyCount)
        setActivityCount(prefs.activityCount)

        gft.linkedChanged.connect(linkInfo.show)
    }

    Component.onDestruction: {
        detector.running = false
        if (unsavedCount) {
            logger.log(unsavedCount, {})
            unsavedCount = 0
        }
    }
}
