import QtQuick 1.1
import "symbian"

StepsPage {
    id: actionsPage
    property bool dialogOpen: false
    property int newActivity: 0
    property variant buttons: []

    Column {
        anchors.centerIn: parent
        spacing: 32
        width: actionsPage.width

        StepsButtonColumn {
            id: buttons
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            BigButton {
                id: activity0
                anchors.left: parent.left; anchors.right: parent.right
                text: main.activityNames[0]
                onClicked: setActivity(0)
            }
            BigButton {
                id: activity2
                anchors.left: parent.left; anchors.right: parent.right
                text: main.activityNames[2]
                onClicked: setActivity(2)
            }
            BigButton {
                id: activity3
                anchors.left: parent.left; anchors.right: parent.right
                text: main.activityNames[3]
                onClicked: setActivity(3)
            }
        }

        BigRedButton {
            text: qsTr("Reset step count")
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                dialogOpen = true
                confirmResetActivityDialog.open()
            }
        }
    }

    StepsYesNoDialog {
        id: confirmResetActivityDialog
        title: qsTr("Reset current activity step count?")
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
        title: qsTr("Change activity to ") + main.activityNames[newActivity] + qsTr("?")
        onDialogAccepted: {
            main.setActivity(newActivity)
            main.pageStack.pop()
        }
        onDialogRejected: {
            onActivityChanged()
        }
        onDialogClosed: {
            dialogOpen = false;
        }
    }

    SettingsPage {
        id: settingsPage
    }

    StepsToolBarLayout {
        id: actionsTools
        StepsToolIcon {
            iconId: "toolbar-back"
            onClicked: back()
        }
        StepsToolIcon {
            iconId: "toolbar-settings"
            onClicked: main.pageStack.push(settingsPage)
        }
        StepsToolIcon {
            iconId: (platform.osName === "symbian")? ":/images/about.png": "toolbar-new-message"
            onClicked: main.pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
        }
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

    function onActivityChanged() {
        if (main.activity === 0 || main.activity === 1)
            buttons.checkedButton = activity0
        else if (main.activity === 2)
            buttons.checkedButton = activity2
        else
            buttons.checkedButton = activity3
    }

    onBack: {
        main.pageStack.pop()
    }

    Component.onCompleted: {
        if (platform.osName === "meego") {
            mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
        }
        main.activityChanged.connect(onActivityChanged)
        onActivityChanged()
        setToolBar(actionsTools)
    }

    onStatusChanged: {
        onActivityChanged()
    }
}
