import QtQuick 1.1
import com.nokia.symbian 1.1

Page {
    property bool active

    active: status == PageStatus.Active

    onStatusChanged: {
        // Enforce focus on activation
        if (status == PageStatus.Active) {
            focus = true
        }
    }
}
