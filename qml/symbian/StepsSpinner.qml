import QtQuick 1.1
import com.nokia.symbian 1.1

// Simple wrapper for BusyIndicator
Dialog {
    property bool running: false
    id: spinner
    opacity: 0.5

    onRunningChanged: {
        if (running) {
            open()
        } else {
            close()
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        width: 150
        height: 150
        running: spinner.running
    }
}

