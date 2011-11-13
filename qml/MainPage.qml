import QtQuick 1.1
import "symbian"
import "meego"

StepsPage {
    property bool counterWasRunning

    StepsLabel {
        id: label
        anchors {
            centerIn: parent
            bottomMargin: 100
        }
        text: counter.count
        font.pixelSize: 158
        font.bold: true
    }

    StepsLabel {
        id: pausedLabel
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: label.bottom
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

    SettingsPage {
        id: settings
        onClosed: {
            console.log("* MainPage.SettingsPage.onClosed")
            counter.running = counterWasRunning
        }
    }

    Beep {
        id: startSound
        source: "/sounds/start.wav"
    }

    Beep {
        id: stopSound
        source: "/sounds/stop.wav"
    }

    Beep {
        id: settingsSound
        source: "/sounds/settings.wav"
    }

   Component.onCompleted: {
        mediaKey.volumeUpPressed.connect(onVolumeUpPressed)
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
    }

    function onVolumeUpPressed() {
        console.log("* MainPage.onVolumeUpPressed")
        var sound = counter.running? stopSound: startSound
        sound.play()
        counter.running = !counter.running
    }

    function onVolumeDownPressed() {
        console.log("* MainPage.onVolumeDownPressed")
        counterWasRunning = counter.running
        counter.running = false
        settingsSound.play()
        settings.open()
    }

    // Keys.onPressed: {
    //     if (event.key == Qt.Key_VolumeUp) {
    //         var sound = counter.running? stopSound: startSound
    //         sound.play()
    //         counter.running = !counter.running
    //     } else if (event.key == Qt.Key_VolumeDown) {
    //         counterWasRunning = counter.running
    //         counter.running = false
    //         settingsSound.play()
    //         settings.open()
    //     }
    // }
}
