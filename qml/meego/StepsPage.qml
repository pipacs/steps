import QtQuick 1.1
import com.nokia.meego 1.0

// Simple wrapper for Page
Page {
    property bool active

    active: status == PageStatus.Active

    onStatusChanged: {
        // Enforce focus on activation
        if (status == PageStatus.Active) {
            console.log("* Page.onStatusChanged: Grabbing focus")
            focus = true
        }
    }
}
