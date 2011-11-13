import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    focus: true
    showStatusBar: false
    Component.onCompleted: {
        theme.inverted = true
    }
}
