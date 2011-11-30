import QtQuick 1.1
import "meego"

StepsPage {
    property bool counterWasRunning: false

    StepsLabel {
        id: dailyLabel
        anchors {
            centerIn: parent
            bottomMargin: 200
        }
        text: main.dailyCount
        font.pixelSize: 116
        font.bold: true
    }

    StepsLabel {
        id: totalLabel
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: dailyLabel.bottom
            topMargin: 37
        }
        text: "Total: " + counter.count
        horizontalAlignment: Text.AlignHCenter
    }

    StepsLabel {
        id: pausedLabel
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: totalLabel.bottom
            topMargin: 37
        }
        width: parent.width
        text: qsTr("Paused")
        font.pixelSize: 45
        color: "#ff9999"
        visible: !counter.running
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    }

    StepsLabel {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: pausedLabel.bottom
            topMargin: 37
        }
        width: parent.width
        text: qsTr("Press Volume Up to start/pause counter, Volume Down to show settings")
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    }

    Beep {
        id: startSound
        source: platform.soundUrl("start")
    }

    Beep {
        id: stopSound
        source: platform.soundUrl("stop")
    }

    Beep {
        id: settingsSound
        source: platform.soundUrl("settings")
    }

    ActionsPage {
        id: actions
    }

    Component.onCompleted: {
        mediaKey.volumeUpPressed.connect(onVolumeUpPressed)
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
    }

    onActiveChanged: {
        console.log("* MainPage: onActiveChanged: " + active)
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
            console.log("* MainPage.onVolumeUpPressed")
            counter.running = !counter.running
            var sound = counter.running? startSound: stopSound
            sound.play()
        }
    }

    function onVolumeDownPressed() {
        if (active) {
            console.log("* MainPage.onVolumeDownPressed")
            settingsSound.play()
            main.pageStack.push(actions)
        }
    }
}
