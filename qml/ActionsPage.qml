import QtQuick 1.1
import "symbian"

StepsPage {
    property bool dialogOpen: false
    id: actionsPage

    Column {
        anchors.centerIn: parent
        spacing: 32
        width: actionsPage.width

        BigButton {
            text: "Reset activity"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            negative: true
            onClicked: {
                dialogOpen = true
                confirmResetActivityDialog.open()
            }
        }

        BigButton {
            text: "Reset all"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            negative: true
            onClicked: {
                dialogOpen = true
                confirmResetDialog.open()
            }
        }

        BigButton {
            text: "Settings"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                main.pageStack.push(settings)
            }
        }
    }

    StepsYesNoDialog {
        id: confirmResetDialog
        titleText: "Reset all step counts?"
        onDialogAccepted: {
            console.log("* ActionsPage.confirmDialog.onDialogAccepted")
            main.resetCount()
            main.pageStack.pop()
        }
        onDialogClosed: {
            dialogOpen = false;
        }
    }

    StepsYesNoDialog {
        id: confirmResetActivityDialog
        titleText: "Reset current activity step count?"
        onDialogAccepted: {
            main.resetActivityCount()
            main.pageStack.pop()
        }
        onDialogClosed: {
            dialogOpen = false;
        }
    }

    SettingsPage {id: settings}

    onBack: {
        console.log("* ActionsPage.onBack")
        main.pageStack.pop()
    }

    function onVolumeDownPressed() {
        if (active && !dialogOpen) {
            console.log("* ActionsPage.onVolumeDownPressed")
            main.pageStack.pop()
        }
    }

    Component.onCompleted: {
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
    }
}
