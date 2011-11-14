import QtQuick 1.1
import "symbian"
import "meego"

StepsDialog {
    id: settings

    acceptText: "Save"
    rejectText: "Cancel"

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
                stepSize: 10
                valueIndicatorVisible: true
                minimumValue: 10
                maximumValue: 190
                value: counter.sensitivity
            }

            StepsCheckBox {
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