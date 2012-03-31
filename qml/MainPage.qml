import QtQuick 1.1
import "meego"

StepsPage {
    property bool detectorWasRunning: false
    id: mainPage
    showBack: prefs.showExit
    showTools: prefs.showExit

    // Upload indicator
    Image {
        source: "qrc:/images/reset.png"
        width: 28
        height: 28
        anchors.top: mainPage.top
        anchors.right: mainPage.right
        visible: uploader.uploading
    }

    Column {
        anchors.centerIn: parent
        spacing: 32
        width: mainPage.width

        // Current activity step count
        StepsLabel {
            id: activityLabel
            y: screen.height / 3
            anchors.horizontalCenter: parent.horizontalCenter
            text: main.activityCount
            font.pixelSize: 116
            font.bold: true
        }

        // Daily step count
        StepsLabel {
            id: dailyLabel
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Today: ") + main.dailyCount
            horizontalAlignment: Text.AlignHCenter
        }

        // Total step count
        StepsLabel {
            id: totalLabel
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Total: ") + main.totalCount
            horizontalAlignment: Text.AlignHCenter
        }

        // Name of current activity
        StepsLabel {
            id: pausedLabel
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: main.activityNames[main.activity] + (detector.running? "": qsTr(" (Paused)"))
            font.pixelSize: 45
            color: "#ff9999"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }

        // Help text
        StepsLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Press Volume Up to start/pause activity, Volume Down to show more options")
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }
    }

    Beep {
        id: startSound
        source: platform.soundUrl("start")
    }

    Beep {
        id: stopSound
        source: platform.soundUrl("stop")
    }

    function onCounterRunningChanged() {
        var sound = detector.running? startSound: stopSound
        sound.beep()
    }

    Component.onCompleted: {
        mediaKey.volumeUpPressed.connect(onVolumeUpPressed)
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
        detector.runningChanged.connect(onCounterRunningChanged)
    }

    onActiveChanged: {
        if (active) {
            detector.running = detectorWasRunning
        } else {
            detectorWasRunning = detector.running
            detector.running = false
        }
    }

    onBack: {
        Qt.quit()
    }

    function onActivityChanged() {
        pausedLabel.text = main.activityNames[main.activity] + (detector.running? "": qsTr(" (Paused)"))
    }

    function onVolumeUpPressed() {
        if (active) {
            detector.running = !detector.running
        }
    }

    function onVolumeDownPressed() {
        if (active) {
            main.pageStack.push(Qt.resolvedUrl("ActionsPage.qml"))
        }
    }
}
