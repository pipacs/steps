import QtQuick 1.1
import com.nokia.meego 1.0

// Simple wrapper for PageStackWindow
PageStackWindow {
    focus: true
    showStatusBar: false
    showToolBar: false
    Component.onCompleted: {
        // Enforce black theme
        theme.inverted = true
    }
}
