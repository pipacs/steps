import QtQuick 1.1
import com.nokia.meego 1.0

// Simple wrapper for Page
Page {
    property bool active
    property bool lockOrientation: true
    property bool showBack: false
    signal back

    active: status == PageStatus.Active
    orientationLock: lockOrientation? PageOrientation.LockPortrait: PageOrientation.Automatic
    tools: pageTools

    ToolBarLayout {
        id: pageTools
        visible: showBack
        ToolIcon {
            iconId: "toolbar-back";
            onClicked: back()
            visible: showBack
        }
    }
}
