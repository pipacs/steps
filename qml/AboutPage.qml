import QtQuick 1.1
import "meego"

StepsPage {
    id: aboutPage

    Flickable {
        anchors.fill: parent
        anchors.margins: 15
        contentWidth: Math.max(parent.width, column.width)
        contentHeight: Math.max(parent.height, column.height)
        flickableDirection: Flickable.VerticalFlick
        id: about

        Column {
            id: column
            width: aboutPage.width - 30
            Image {
                source: "qrc:/images/steps.png"
            }
            StepsLabel {
                width: parent.width
                font.pixelSize: (platform.osName === "harmattan")? 30: 26
                textFormat: Text.RichText
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("<b>Steps</b><p>Version ") + platform.appVersion + qsTr("<p>") + qsTr("Device ID: ") + platform.deviceId + qsTr("</p>") + platform.text("about.html")
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }

    StepsScrollDecorator {
        flickableItem: about
    }

    StepsBanner {
        id: info
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPressAndHold: {
            prefs.traceToFile = !prefs.traceToFile
            platform.traceToFile(prefs.traceToFile)
            if (prefs.traceToFile) {
                var now = new Date();
                console.log("* Steps version " + platform.appVersion + ", " + now.toString())
            }
            info.text = "Tracing " + (prefs.traceToFile? "enabled": "disabled")
            info.show()
        }
        onDoubleClicked: {
            platform.deleteTraceFile()
            prefs.traceToFile = false
            info.text = "Tracing disabled, trace file deleted"
            info.show()
        }
    }

    onBack: main.pageStack.pop()
}
