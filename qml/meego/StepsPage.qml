import QtQuick 1.1
import com.nokia.meego 1.0

// Simple wrapper for Page
Page {
    onStatusChanged: {
        // Enforce focus on activation
        if (status == PageStatus.Active) {
            console.log("* Page.onStatusChanged: Grabbing focus")
            focus = true
        }
    }
}
