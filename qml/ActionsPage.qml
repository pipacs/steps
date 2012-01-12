import QtQuick 1.1
import "symbian"

StepsPage {
    property bool dialogOpen: false
    property int newActivity: 0
    id: actionsPage

    Column {
        anchors.centerIn: parent
        spacing: 32
        width: actionsPage.width

        StepsButtonColumn {
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            BigButton {
                id: activity0
                anchors.left: parent.left; anchors.right: parent.right
                text: main.activityNames[0]
                checked: main.activity === 0
                onClicked: setActivity(0)
            }
            BigButton {
                id: activity1
                anchors.left: parent.left; anchors.right: parent.right
                text: main.activityNames[1]
                checked: main.activity === 1
                onClicked: setActivity(1)
            }
            BigButton {
                id: activity2
                anchors.left: parent.left; anchors.right: parent.right
                text: main.activityNames[2]
                checked: main.activity === 2
                onClicked: setActivity(2)
            }
            BigButton {
                id: activity3
                anchors.left: parent.left; anchors.right: parent.right
                text: main.activityNames[3]
                checked: main.activity === 3
                onClicked: setActivity(3)
            }
        }

        BigRedButton {
            text: "Reset step count"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                dialogOpen = true
                confirmResetActivityDialog.open()
            }
        }

        BigButton {
            text: "Settings"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                main.pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
        }
    }

    StepsYesNoDialog {
        id: confirmResetActivityDialog
        title: "Reset current activity step count?"
        onDialogAccepted: {
            main.resetActivityCount()
            main.pageStack.pop()
        }
        onDialogClosed: {
            dialogOpen = false;
        }
    }

    StepsYesNoDialog {
        id: confirmChangeActivityDialog
        title: "Change activity to " + main.activityNames[newActivity] + "?"
        onDialogAccepted: {
            main.setActivity(newActivity)
            main.pageStack.pop()
        }
        onDialogRejected: {
            activity0.checked = (main.activity === 0)
            activity1.checked = (main.activity === 1)
            activity2.checked = (main.activity === 2)
            activity3.checked = (main.activity === 3)
        }
        onDialogClosed: {
            dialogOpen = false;
        }
    }

    onBack: {
        main.pageStack.pop()
    }

    function setActivity(a) {
        if (a !== main.activity) {
            newActivity = a
            dialogOpen = true
            confirmChangeActivityDialog.open()
        }
    }

    function onVolumeDownPressed() {
        if (active && !dialogOpen) {
            main.pageStack.pop()
        }
    }

    Component.onCompleted: {
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
    }
}
