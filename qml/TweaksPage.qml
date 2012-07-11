import QtQuick 1.1
import "meego"

StepsPage {
    id: tweaks

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
            width: tweaks.width - 60

            StepsLabel {text: qsTr("Save to Quantis:")}

            StepsCheckBox {
                text: qsTr("Enable saving")
                id: enableSharing
                checked: qc.enabled
                enabled: qc.linked
            }

            StepsButton {
                text: qc.linked? qsTr("Logout from Quantis"): qsTr("Login to Quantis")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    if (qc.linked) {
                        confirmLogoutDialog.open()
                    } else {
                        spinner.running = true
                        qc.link()
                    }
                }
            }

            Gap {}

            StepsLabel {
                text: qsTr("Tracing:")
            }

            StepsCheckBox {
                id: traceToFile
                text: qsTr("Trace to file")
                checked: prefs.traceToFile
                onClicked: {
                    prefs.traceToFile = !prefs.traceToFile
                    platform.traceToFile(prefs.traceToFile)
                    if (prefs.traceToFile) {
                        var now = new Date();
                        console.log("* Steps version " + platform.appVersion + ", " + now.toString())
                    }
                    info.text = "Tracing " + (prefs.traceToFile? "enabled": "disabled")
                    info.show()
                }
            }

            StepsButton {
                id: clearTrace
                text: qsTr("Delete trace file")
                onClicked: {
                    platform.deleteTraceFile()
                    prefs.traceToFile = false
                    traceToFile.checked = false
                    info.text = "Tracing disabled, trace file deleted"
                    info.show()
                }
            }

            Gap {}

            StepsLabel {
                text: qsTr("Minimum time between steps (ms):")
            }

            StepsSlider {
                id: runningDiff
                width: flickable.width - 15
                stepSize: 50
                valueIndicatorVisible: true
                minimumValue: 50
                maximumValue: 350
                value: detector.runningStepTimeDiff
            }

            Gap {}

            StepsLabel {
                text: qsTr("Minimum reading delta:")
            }

            StepsSlider {
                id: readingDiff
                width: flickable.width - 15
                stepSize: 10
                valueIndicatorVisible: true
                minimumValue: 10
                maximumValue: 60
                value: detector.minReadingDiff
            }
        }
    }

    StepsScrollDecorator {
        flickableItem: flickable
    }

    StepsBanner {
        id: info
        text: qc.linked? qsTr("Logged in to Quantis"): qsTr("Logged out from Quantis:")
    }

    StepsYesNoDialog {
        id: confirmLogoutDialog
        titleText: qsTr("Are you sure to log out?")
        onDialogAccepted: {
            qc.unlink()
        }
    }

    StepsSpinner {
        id: spinner
    }

    QcLoginBrowser {
        id: loginBrowser
    }

    function openBrowser(url) {
        spinner.running = false
        main.pageStack.push(loginBrowser)
        loginBrowser.openUrl(url)
    }

    function onLinkedChanged() {
        enableSharing.enabled = qc.linked
    }

    function onLinkingFailed() {
        console.log("* onLinkingFailed")
    }

    onBack: {
        main.pageStack.pop()
        detector.runningStepTimeDiff = runningDiff.value
        prefs.runningStepTimeDiff = runningDiff.value
        detector.minReadingDiff = readingDiff.value
        prefs.minReadingDiff = readingDiff.value
    }

    Component.onCompleted: {
        qc.openBrowser.connect(openBrowser);
        qc.linkedChanged.connect(onLinkedChanged)
        qc.linkingFailed.connect(onLinkingFailed)    }
}
