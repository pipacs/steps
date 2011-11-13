import QtQuick 1.1
import com.nokia.symbian 1.1

CommonDialog {
    id: settings

    titleText: "Settings"
    buttonTexts: ["Save", "Cancel"]
    anchors.fill: parent

    content: Flickable {
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.topMargin: 30
        contentWidth: col2.width
        contentHeight: col2.height
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: col2
            anchors.top: parent.top
            spacing: 41
            width: parent.width

            CheckBox {
                id: audioFeedback
                text: "Sound effects"
                checked: !prefs.muted
            }

            Label {
                id: calibrationLabel
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: "Calibration: Measured " + counter.count + " steps,\nbut it should be:"
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

    onButtonClicked: {
        if (index === 0) {
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
            resetCounter.checked = false
        }
    }
}
