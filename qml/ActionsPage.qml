import QtQuick 1.1
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
                    spinner.visible = true
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

    BigButton {
        text: "Back"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: actionsPage.width - 64
        onClicked: appWindow.pageStack.pop()
    }

    StepsYesNoDialog {
        id: confirmResetDialog
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
        onDialogAccepted: appWindow.pageStack.pop()
        onDialogClosed: dialogOpen = false
    }

    LoginBrowser {
        id: loginBrowser
    }

    StepsSpinner {
        id: spinner
        running: false
        visible: false
    }

    function onVolumeDownPressed() {
        if (active && !dialogOpen) {
            console.log("* ActionsPage.onVolumeDownPressed")
            appWindow.pageStack.pop()
        }
    }

    function openUrl(url) {
        console.log("* ActionsPage.openUrl " + url)
        spinner.running = false
        spinner.visible = false
        loginBrowser.url = url
        appWindow.pageStack.push(loginBrowser)
    }

    function onLinkingSucceeded() {
        console.log("* ActionsPage.onLinkingSucceeded")
    }

    function onLinkingFailed(error) {
        console.log("* ActionsPage.onLinkingFailed " + error)
    }

    Component.onCompleted: {
        mediaKey.volumeDownPressed.connect(onVolumeDownPressed)
        googleDocs.openUrl.connect(openUrl);
        googleDocs.linkingSucceeded.connect(onLinkingSucceeded)
        googleDocs.linkingFailed.connect(onLinkingFailed)
    }
}
