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

            StepsLabel {
                id: calibrationLabel
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: "Calibration: Measured " + counter.count + " steps, but it should be:"
                enabled: counter.count
            }

            StepsSlider {
                id: calibrationSlider
                width: parent.width - 15
                enabled: counter.count
                stepSize: 1
                valueIndicatorVisible: true
                minimumValue: Math.ceil(counter.count - counter.count / 3)
                maximumValue: Math.ceil(counter.count + counter.count / 3)
                value: counter.count
            }

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

            StepsLabel {text: "Sharing:"}

            StepsCheckBox {
                text: "Share to Google Documents"
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

    onBack: {
        console.log("* SettingsPage.onBack")
        main.pageStack.pop()
        if (calibrationSlider.changed && counter.rawCount) {
            console.log("*  New calibration value " + calibrationSlider.value)
            counter.calibration = calibrationSlider.value / counter.rawCount
            prefs.calibration = counter.calibration
        }
        prefs.muted = !audioFeedback.checked
        counter.setSensitivity(sensitivitySlider.value)
        prefs.sensitivity = counter.sensitivity
        gft.enabled = enableSharing.checked
    }

    function openBrowser(url) {
        console.log("* SettingsPage.openBrowser " + url)
        spinner.running = false
        main.pageStack.push(loginBrowser)
        loginBrowser.openUrl(url)
    }

    Component.onCompleted: {
        gft.openBrowser.connect(openBrowser);
    }
}
