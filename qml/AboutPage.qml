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
        onPressAndHold: main.pageStack.push(Qt.resolvedUrl("TweaksPage.qml"))
    }

    onBack: main.pageStack.pop()
}
