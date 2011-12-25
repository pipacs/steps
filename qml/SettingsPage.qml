import QtQuick 1.1
import "meego"

StepsPage {
    id: settings

    Flickable {
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.rightMargin: 30
        anchors.topMargin: 41
        contentWidth: col2.width
        contentHeight: col2.height
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: col2
            anchors.top: parent.top
            spacing: 31
            width: settings.width - 60

            StepsCheckBox {
                id: audioFeedback
                text: "Sound effects"
                checked: !prefs.muted
            }

            //
            // StepsLabel {
            //     id: calibrationLabel
            //     width: parent.width
            //     wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            //     text: "Calibration: Measured " + main.activityCount + " steps, but it should be:"
            //     enabled: main.activityCount
            // }

            // StepsSlider {
            //     id: calibrationSlider
            //     width: parent.width - 15
            //     enabled: main.activityCount
            //     stepSize: 1
            //     valueIndicatorVisible: true
            //     minimumValue: Math.ceil(main.activityCount - main.activityCount / 3)
            //     maximumValue: Math.ceil(main.activityCount + main.activityCount / 3)
            //     value: main.activityCount
            // }

            StepsLabel {text: "Sensitivity:"}

            StepsSlider {
                id: sensitivitySlider
                width: parent.width - 15
                stepSize: 10
                valueIndicatorVisible: true
                minimumValue: 10
                maximumValue: 190
                value: counter.sensitivity
            }

            StepsLabel {text: "Save to Google Docs:"}

            StepsCheckBox {
                text: "Enable saving"
                id: enableSharing
                checked: gft.enabled
                enabled: gft.linked
            }

            StepsButton {
                text: gft.linked? "Logout from Google": "Login to Google"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    if (gft.linked) {
                        dialogOpen = true
                        confirmLogoutDialog.open()
                    } else {
                        spinner.running = true
                        gft.link()
                    }
                }
            }
        }
    }

    StepsYesNoDialog {
        id: confirmResetAllDialog
        titleText: "Reset all settings?"
        onDialogAccepted: {
            prefs.muted = true
            audioFeedback.checked = false
            counter.resetSettings()
            sensitivitySlider.value = counter.sensitivity
            gft.enabled = false
            enableSharing.checked = false
        }
    }

    StepsYesNoDialog {
        id: confirmLogoutDialog
        titleText: "Are you sure to log out?"
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
        text: gft.linked? "Logged in to Google Docs": "Logged out from Google Docs"
    }

    onBack: {
        console.log("* SettingsPage.onBack")
        main.pageStack.pop()
        // if (calibrationSlider.changed && counter.rawCount) {
        //     var activityDelta = calibrationSlider.value - main.activityCount
        //     counter.calibration = (counter.count + activityDelta) / counter.rawCount
        //     prefs.calibration = counter.calibration
        // }
        prefs.muted = !audioFeedback.checked
        counter.setSensitivity(sensitivitySlider.value)
        prefs.sensitivity = counter.sensitivity
        gft.enabled = enableSharing.checked
    }

    function openBrowser(url) {
        spinner.running = false
        main.pageStack.push(loginBrowser)
        loginBrowser.openUrl(url)
    }

    Component.onCompleted: {
        gft.openBrowser.connect(openBrowser);
        gft.linkedChanged.connect(linkInfo.show)
    }
}
