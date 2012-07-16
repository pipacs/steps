import QtQuick 1.1
import "meego"

StepsPage {
    property bool detectorWasRunning: false
    id: mainPage
    showBack: platform.os === "symbian"
    showTools: platform.os === "symbian"

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

        // Running/paused label
        StepsLabel {
            id: pausedLabel
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: detector.running? "": qsTr(" (Paused)")
            font.pixelSize: 45
            color: "#ff9999"
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

    StepsToolBarLayout {
        id: myTools
        StepsToolIcon {
            iconId: "toolbar-back"
            onClicked: Qt.quit()
            visible: platform.osName === "symbian"
        }
        StepsToolIcon {
            id: play
            iconId: detector.running? "toolbar-mediacontrol-stop": "toolbar-mediacontrol-play"
            onClicked: detector.running = !detector.running
        }
        StepsToolIcon {
            iconId: "toolbar-settings"
            onClicked: main.pageStack.push(settingsPage)
        }
        StepsToolIcon {
            iconId: (platform.osName === "symbian")? ":/images/about.png": "toolbar-new-message"
            onClicked: main.pageStack.push(aboutPage)
        }
    }

    function onCounterRunningChanged() {
        var sound = detector.running? startSound: stopSound
        sound.beep()
    }

    Component.onCompleted: {
        setToolBar(myTools)
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
}
