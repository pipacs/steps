import QtQuick 1.1
import com.nokia.meego 1.0

// Simple wrapper for Sheet
Sheet {
    property string title
    property string acceptText
    property string rejectText
    signal closed
    signal accepted
    signal rejected

    acceptButtonText: acceptText
    rejectButtonText: rejectText

    onStatusChanged: {
        if (status == DialogStatus.Closed) {
            // emit closed
            closed()
        }
    }

}
