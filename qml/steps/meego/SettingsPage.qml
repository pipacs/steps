import QtQuick 1.1
import com.nokia.meego 1.0

Sheet {
    id: settings

    acceptButtonText: "Save"
    rejectButtonText: "Cancel"

    content: Flickable {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.topMargin: 10
        contentWidth: col2.width
        contentHeight: col2.height
        flickableDirection: Flickable.VerticalFlick

        Column {
            anchors.top: parent.top
            spacing: 30
            CheckBox {
                id: resetCounter
                text: "Reset counter"
                checked: false
            }

            Label {
                id: calibrationLabel
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: "Calibration: Measured " + counter.count + " steps, but it should be:"
                enabled: counter.count
            }

            Slider {
                id: calibrationSlider
                property bool firstUpdate: true
                property bool changed: false
                enabled: counter.count
                stepSize: 1
                valueIndicatorVisible: true
                minimumValue: Math.ceil(counter.count - counter.count / 2)
                maximumValue: Math.ceil(counter.count + counter.count / 2)
                value: counter.count
                onValueChanged: {
                    if (firstUpdate) {
                        // Work around a slider bug: The first valueChanged notification is sent too early
                        firstUpdate = false
                    } else {
                        changed = true
                    }
                }
            }
        }
    }

    onAccepted: {
        if (calibrationSlider.changed) {
            counter.calibration = calibrationSlider.value / counter.rawCount
        }
        if (resetCounter.checked) {
            counter.reset()
        }
    }
}
