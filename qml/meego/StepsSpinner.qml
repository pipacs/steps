import QtQuick 1.1
import com.nokia.meego 1.0

// Simple wrapper for BusyIndicator
BusyIndicator {
    anchors.centerIn: parent
    visible: running
    platformStyle: BusyIndicatorStyle {size: "large"}
}
