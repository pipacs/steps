import QtQuick 1.1
import com.nokia.meego 1.0

// Simple wrapper for PageStackWindow
PageStackWindow {
    property bool toolBar: false

    focus: true
    showStatusBar: false
    showToolBar: toolbar

    Component.onCompleted: {
        // Enforce black theme
        theme.inverted = true
    }
}
