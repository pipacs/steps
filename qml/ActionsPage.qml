import QtQuick 1.1
import "symbian"
import "meego"

StepsPage {
    property bool settingsOpen: false

    Column {
        anchors.centerIn: parent
        spacing: 32
        BigButton {
            text: "Reset counter"
            negative: true
            onClicked: {
                counter.reset()
                appWindow.pageStack.pop()
            }
        }
        BigButton {
            text: "Settings"
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
        onDialogClosed: {
            console.log("* Actions.SettingsPage.onDialogClosed")
            appWindow.pageStack.pop()
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
