import QtQuick 1.1
import com.nokia.symbian 1.1
import QtMultimediaKit 1.1
import MediaKey 1.0

Page {
    property bool counterWasRunning

    Label {
        id: label
        anchors {
            centerIn: parent
            bottomMargin: 100
        }
        text: counter.count
        font.pixelSize: 158
        font.bold: true
    }

    Label {
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

    Label {
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
        onStatusChanged: {
            if (status == DialogStatus.Closed) {
                counter.running = counterWasRunning
            }
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
        source: "/sounds/beep.wav"
    }

    onStatusChanged: {
        if (status == PageStatus.Active) {
            focus = true
        }
    }

    MediaKey {
        onVolumeUpPressed: {
            var sound = counter.running? stopSound: startSound
            sound.play()
            counter.running = !counter.running
        }
        onVolumeDownPressed: {
            counterWasRunning = counter.running
            counter.running = false
            settingsSound.play()
            settings.open()
        }
    }
}
