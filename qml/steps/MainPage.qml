import QtQuick 1.1
import com.nokia.meego 1.0

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
        text: qsTr("Press Volume Up to start/pause counter, Volume Down to reset step count")
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    }

    QueryDialog {
        id: resetQuery
        // icon: FIXME
        titleText: qsTr("Reset step count?")
        acceptButtonText: qsTr("Yes")
        rejectButtonText: qsTr("No")
        onAccepted: {
            counter.reset()
            counter.running = counterWasRunning
        }
    }

    onStatusChanged: {
        if (status == PageStatus.Activating) {
            focus = true
        }
    }

    // Handle up/down keys
    Keys.onPressed: {
        if (event.key == Qt.Key_VolumeUp) {
            counter.running = !counter.running
        } else if (event.key == Qt.Key_VolumeDown) {
            counterWasRunning = counter.running
            resetQuery.open()
        }
    }
}
