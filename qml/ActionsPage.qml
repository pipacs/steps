import QtQuick 1.1
import "symbian"
import "meego"

StepsPage {
    property bool dialogOpen: false
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
                dialogOpen = true
                confirmDialog.open()
            }
        }
        BigButton {
            text: "Settings"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                dialogOpen = true
                settings.open()
            }
        }
    }

    Component.onCompleted: {
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
    }

    StepsYesNoDialog {
        id: confirmDialog
        titleText: "Reset counter?"
        onDialogAccepted: {
            console.log("* ActionsPage.confirmDialog.onDialogAccepted")
            counter.reset()
            appWindow.pageStack.pop()
        }
        onDialogClosed: {
            dialogOpen = false;
        }
    }

    SettingsPage {
        id: settings
        onDialogAccepted: {
            console.log("* Actions.SettingsPage.onDialogAccepted")
            appWindow.pageStack.pop()
        }
        onDialogClosed: {
            console.log("* Actions.SettingsPage.onDialogClosed")
            dialogOpen = false
        }
    }

    function onVolumeDownPressed() {
        if (active && !dialogOpen) {
            console.log("* ActionsPage.onVolumeDownPressed")
            appWindow.pageStack.pop()
        }
    }
}
