import QtQuick 1.1
import "meego"

StepsPage {
    id: settings

    StepsFlickable {
        id: flickable
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: Math.max(parent.height, column.height)
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: column
            anchors.top: parent.top
            spacing: 25
            width: settings.width

            Gap {}

            BigRedButton {
                text: qsTr("Reset step count")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: confirmResetActivityDialog.open()
            }

            Gap {}

            StepsCheckBox {
                id: savePower
                text: qsTr("Conserve power")
                checked: prefs.savePower
            }

            StepsCheckBox {
                id: audioFeedback
                text: qsTr("Sound effects")
                checked: !prefs.muted
            }

            StepsCheckBox {
                id: showExit
                text: qsTr("Show Exit icon")
                checked: prefs.showExit
            }

            Gap {}

            StepsLabel {text: qsTr("Save to Google Docs:")}

            StepsCheckBox {
                text: qsTr("Enable saving")
                id: enableSharing
                checked: gft.enabled
                enabled: gft.linked
            }

            StepsButton {
                text: gft.linked? qsTr("Logout from Google"): qsTr("Login to Google")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    if (gft.linked) {
                        confirmLogoutDialog.open()
                    } else {
                        spinner.running = true
                        gft.link()
                    }
                }
            }
        }
    }

    StepsScrollDecorator {
        flickableItem: flickable
    }

    StepsYesNoDialog {
        id: confirmLogoutDialog
        titleText: qsTr("Are you sure to log out?")
        onDialogAccepted: {
            gft.unlink()
        }
    }

    StepsSpinner {
        id: spinner
    }

    LoginBrowser {
        id: loginBrowser
    }

    StepsYesNoDialog {
        id: confirmResetActivityDialog
        title: qsTr("Reset current step count?")
        onDialogAccepted: {
            main.resetActivityCount()
            main.pageStack.pop()
        }
    }

    onBack: {
        main.pageStack.pop()
        prefs.muted = !audioFeedback.checked
        gft.enabled = enableSharing.checked
        main.activityNames = [main.activityNames[0], main.activityNames[1], custom1Text.text, custom2Text.text]
        prefs.showExit = showExit.checked
        prefs.savePower = savePower.checked
        if (enableSharing.checked && enableSharing.enabled) {
            uploader.upload()
        }
    }

    function openBrowser(url) {
        spinner.running = false
        main.pageStack.push(loginBrowser)
        loginBrowser.openUrl(url)
    }

    function onLinkedChanged() {
        enableSharing.enabled = gft.linked
    }

    Component.onCompleted: {
        gft.openBrowser.connect(openBrowser);
        gft.linkedChanged.connect(onLinkedChanged)
        if (platform.osName !== "symbian") {
            showExit.height = 0
            showExit.visible = false
        }
    }
}
