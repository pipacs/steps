import QtQuick 1.1
import "symbian"
import "meego"

StepsPage {
    property bool settingsOpen: false
    id: stepsPage

    Column {
        anchors.centerIn: parent
        spacing: 32
        width: stepsPage.width
        BigButton {
            text: "Reset counter"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            negative: true
            onClicked: {
                counter.reset()
                appWindow.pageStack.pop()
            }
        }
        BigButton {
            text: "Settings"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                settingsOpen = true
                settings.open()
            }
        }
    }

    Component.onCompleted: {
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
    }

    SettingsPage {
        id: settings
        onDialogAccepted: {
            console.log("* Actions.SettingsPage.onDialogAccepted")
            appWindow.pageStack.pop()
        }
        onDialogClosed: {
            console.log("* Actions.SettingsPage.onDialogClosed")
            settingsOpen = false
        }
    }

    function onVolumeDownPressed() {
        if (active && !settingsOpen) {
            console.log("* ActionsPage.onVolumeDownPressed")
            appWindow.pageStack.pop()
        }
    }
}
