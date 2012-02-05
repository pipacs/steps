import QtQuick 1.1
import "symbian"

StepsPage {
    id: settings

    StepsFlickable {
        id: flickable
        anchors.fill: parent
        anchors.margins: 15
        contentWidth: Math.max(parent.width, column.width)
        contentHeight: Math.max(parent.height, column.height)
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: column
            anchors.top: parent.top
            spacing: 25
            width: settings.width - 60

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

            StepsLabel {text: qsTr("Sensitivity:")}

            StepsSlider {
                id: sensitivitySlider
                width: parent.width - 15
                stepSize: 10
                valueIndicatorVisible: true
                minimumValue: 10
                maximumValue: 190
                value: counter.sensitivity
            }

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

            StepsLabel {
                id: custom1Label
                text: qsTr("Rename activity \"Custom 1\":")
            }

            StepsTextField {
                id: custom1Text
                width: parent.width
                text: main.activityNames[2]
            }

            StepsLabel {
                id: custom2Label
                text: qsTr("Rename activity \"Custom 2\":")
            }

            StepsTextField {
                id: custom2Text
                width: parent.width
                text: main.activityNames[3]
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

    StepsBanner {
        id: linkInfo
        text: gft.linked? qsTr("Logged in to Google Docs"): qsTr("Logged out from Google Docs")
    }

    onBack: {
        main.pageStack.pop()
        prefs.muted = !audioFeedback.checked
        counter.setSensitivity(sensitivitySlider.value)
        prefs.sensitivity = counter.sensitivity
        gft.enabled = enableSharing.checked
        main.activityNames = [main.activityNames[0], main.activityNames[1], custom1Text.text, custom2Text.text]
        prefs.showExit = showExit.checked
        prefs.savePower = savePower.checked
    }

    function openBrowser(url) {
        spinner.running = false
        main.pageStack.push(loginBrowser)
        loginBrowser.openUrl(url)
    }

    Component.onCompleted: {
        gft.openBrowser.connect(openBrowser);
        gft.linkedChanged.connect(linkInfo.show)
        if (platform.osName !== "symbian") {
            showExit.height = 0
            showExit.visible = false
//        } else {
//            custom1Label.height = 0
//            custom1Label.visible = 0
//            custom1Text.height = 0
//            custom1Text.visible = 0
//            custom2Label.height = 0
//            custom2Label.visible = 0
//            custom2Text.height = 0
//            custom2Text.visible = 0
        }
    }
}
