import QtQuick 1.1
import "symbian"

StepsPage {
    property bool dialogOpen: false

    id: actionsPage
    showBack: true

    Column {
        anchors.centerIn: parent
        spacing: 32
        width: actionsPage.width

        BigButton {
            text: "Reset daily"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            negative: true
            onClicked: {
                dialogOpen = true
                confirmResetDailyDialog.open()
            }
        }

        BigButton {
            text: "Reset total"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            negative: true
            onClicked: {
                dialogOpen = true
                confirmResetDialog.open()
            }
        }

        BigButton {
            text: googleDocs.linked? "Stop sharing": "Share"
            width: parent.width - 64
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                if (googleDocs.linked) {
                    dialogOpen = true
                    confirmLogoutDialog.open()
                } else {
                    spinner.running = true
                    googleDocs.link()
                }
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

    StepsYesNoDialog {
        id: confirmResetDialog
        titleText: "Reset total step count?"
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
        id: confirmResetDailyDialog
        titleText: "Reset daily step count?"
        onDialogAccepted: {
            console.log("* ActionsPage.confirmDialog.onDialogAccepted")
            main.resetDailyCount()
            main.pageStack.pop()
        }
        onDialogClosed: {
            dialogOpen = false;
        }
    }

    StepsYesNoDialog {
        id: confirmLogoutDialog
        titleText: "Are you sure to stop sharing?"
        onDialogAccepted: {
            googleDocs.unlink()
        }
        onDialogClosed: {
            dialogOpen = false
        }
    }

    SettingsPage {
        id: settings
        onDialogClosed: dialogOpen = false
    }

    LoginBrowser {
        id: loginBrowser
    }

    StepsSpinner {
        id: spinner
    }

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

    function openUrl(url) {
        console.log("* ActionsPage.openUrl " + url)
        spinner.running = false
        main.pageStack.push(loginBrowser)
        loginBrowser.openUrl(url)
    }

    Component.onCompleted: {
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
        googleDocs.openUrl.connect(openUrl);
    }
}
