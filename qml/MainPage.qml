import QtQuick 1.1
import "symbian"

StepsPage {
    property bool counterWasRunning: false
    id: mainPage
    showBack: false

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

        StepsLabel {
            id: dailyLabel
            y: screen.height / 3
            anchors.horizontalCenter: parent.horizontalCenter
            text: main.dailyCount
            font.pixelSize: 116
            font.bold: true
        }

        StepsLabel {
            id: totalLabel
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Total: " + counter.count
            font.pixelSize: 45
            horizontalAlignment: Text.AlignHCenter
        }

        StepsLabel {
            id: pausedLabel
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: counter.running? "": qsTr("Paused")
            font.pixelSize: 45
            color: "#ff9999"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }

        StepsLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Press Volume Up to start/pause counter, Volume Down to show settings")
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
