import QtQuick 1.1
import com.nokia.symbian 1.1

Page {
    onStatusChanged: {
        // Enforce focus on activation
        if (status == PageStatus.Active) {
            focus = true
        }
    }
}
