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
            width: aboutPage.width
            Image {
                source: "qrc:/images/steps.png"
            }
            StepsLabel {
                width: parent.width
                font.pixelSize: platform.osName === "harmattan"? 32: 26
                textFormat: Text.RichText
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("<b>Steps</b><p>Version ") + platform.appVersion + qsTr("<p>") + platform.text("about.html")
                onLinkActivated: platform.browse(link)
            }
        }
    }

    StepsScrollDecorator {
        flickableItem: about
    }

    onBack: {
        main.pageStack.pop()
    }
}
