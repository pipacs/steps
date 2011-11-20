import QtQuick 1.1
import "symbian"
import "meego"

StepsPage {
    property bool dialogOpen: false
    id: actionsPage

    Column {
        anchors.centerIn: parent
        spacing: 32
        width: actionsPage.width
        BigButton {
            text: "Reset counter"
            // iconSource: "/images/reset.png"
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
            // iconSource: "/images/settings.png"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                dialogOpen = true
                settings.open()
            }
        }
    }

    BigButton {
        text: "Back"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: actionsPage.width - 64
        onClicked: appWindow.pageStack.pop()
    }

    Component.onCompleted: mediaKey.volumeDownPressed.connect(onVolumeDownPressed)

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
        onDialogAccepted: appWindow.pageStack.pop()
        onDialogClosed: dialogOpen = false
    }

    function onVolumeDownPressed() {
        if (active && !dialogOpen) {
            console.log("* ActionsPage.onVolumeDownPressed")
            appWindow.pageStack.pop()
        }
    }
}
