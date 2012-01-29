import QtQuick 1.1
import com.nokia.meego 1.0

// Simple wrapper for Page
Page {
    property bool active
    property bool lockOrientation: true
    property bool showBack: true
    property bool showTools: true
    signal back

    active: status === PageStatus.Active
    orientationLock: lockOrientation? PageOrientation.LockPortrait: PageOrientation.Automatic
    tools: showTools? pageTools: null

    ToolBarLayout {
        id: pageTools
        ToolIcon {
            iconId: "toolbar-back";
            onClicked: back()
            visible: showBack
        }
    }

    Component.onCompleted: {
        console.log("* StepsPage.pageTools children[0]: " + pageTools.children[0].iconId)
    }

    function setToolBar(newToolBar) {
        pageTools.visible = false
        tools = newToolBar
        return pageTools
    }
}
