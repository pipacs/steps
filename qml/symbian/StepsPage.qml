import QtQuick 1.1
import com.nokia.symbian 1.1

Page {
    property bool active
    property bool showBack: false
    property bool lockOrientation: true
    signal back
    orientationLock: lockOrientation? PageOrientation.LockPortrait: PageOrientation.Automatic

    active: status === PageStatus.Active

    tools: pageTools

    ToolBarLayout {
        id: pageTools
        visible: showBack
        ToolButton {
            // "Back"/"Exit" is always visible on Symbian as some phones don't have hard keys to exit the app
            // visible: showBack

            // There is no toolbar-exit icon, so let's stick to toolbar-back for now
            // iconSource: (pageStack.depth > 1)? "toolbar-back": "toolbar-back"
            iconSource: "toolbar-back"

            flat: true
            onClicked: back()
        }
    }

    onStatusChanged: {
        // Enforce focus on activation
        if (status === PageStatus.Active) {
            focus = true
        }
    }
}
