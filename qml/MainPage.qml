import QtQuick 1.1
import "symbian"

StepsPage {
    property bool counterWasRunning: false
    id: mainPage
    showBack: false

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
            text: "Today: " + main.dailyCount
            horizontalAlignment: Text.AlignHCenter
        }

        // Total step count
        StepsLabel {
            id: totalLabel
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Total: " + counter.count
            horizontalAlignment: Text.AlignHCenter
        }

        // Name of current activity
        StepsLabel {
            id: pausedLabel
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: main.activityName(main.activity) + (counter.running? "": qsTr(" (Paused)"))
            font.pixelSize: 45
            color: "#ff9999"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }

        // Help text
        StepsLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Press Volume Up to start/pause activity, Volume Down to show settings")
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
        var sound = counter.running? startSound: stopSound
        sound.beep()
    }

    Component.onCompleted: {
        mediaKey.volumeUpPressed.connect(onVolumeUpPressed)
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
        counter.runningChanged.connect(onCounterRunningChanged);
    }

    onActiveChanged: {
        if (active) {
            counter.running = counterWasRunning
        } else {
            counterWasRunning = counter.running
            counter.running = false
        }
    }

    onBack: {
        Qt.quit()
    }

    function onVolumeUpPressed() {
        if (active) {
            counter.running = !counter.running
        }
    }

    function onVolumeDownPressed() {
        if (active) {
            main.pageStack.push(Qt.resolvedUrl("ActionsPage.qml"))
        }
    }
}
