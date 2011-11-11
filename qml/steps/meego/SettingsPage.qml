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
            id: col2
            anchors.top: parent.top
            spacing: 41

            CheckBox {
                id: audioFeedback
                text: "Sound effects"
                checked: !prefs.muted
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
                minimumValue: Math.ceil(counter.count - counter.count / 3)
                maximumValue: Math.ceil(counter.count + counter.count / 3)
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

            Label {text: "Sensitivity:"}

            Slider {
                id: sensitivitySlider
                stepSize: 10
                valueIndicatorVisible: true
                minimumValue: 10
                maximumValue: 190
                value: counter.sensitivity
            }

            CheckBox {
                id: resetCounter
                text: "Reset counter"
                checked: false
            }
        }
    }

    onAccepted: {
        if (calibrationSlider.changed && counter.rawCount) {
            counter.calibration = calibrationSlider.value / counter.rawCount
            prefs.calibration = counter.calibration
        }
        if (resetCounter.checked) {
            counter.reset()
            resetCounter.checked = false
        }
        prefs.muted = !audioFeedback.checked
        counter.setSensitivity(sensitivitySlider.value)
        prefs.sensitivity = counter.sensitivity
    }

    onRejected: {
        resetCounter.checked = false
    }
}
