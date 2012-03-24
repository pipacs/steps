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

            StepsLabel {
                text: qsTr("Step detector:")
            }

            StepsButtonColumn {
                StepsButton {
                    id: brutal
                    text: qsTr("Brutal")
                }
                StepsButton {
                    id: finessa
                    text: qsTr("Finessa")
                }
                StepsButton {
                    id: runningDane
                    text: qsTr("Running Dane")
                }
            }
        }
    }

    StepsScrollDecorator {
        flickableItem: flickable
    }

    StepsBanner {
        id: info
        text: gft.linked? qsTr("Logged in to Google Docs"): qsTr("Logged out from Google Docs")
    }

    onBack: {
        main.pageStack.pop()
    }
}
