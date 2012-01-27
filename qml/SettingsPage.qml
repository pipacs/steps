import QtQuick 1.1
import "meego"

StepsPage {
    id: settings

    Flickable {
        id: flickable
        anchors {left: parent.left; right: parent.right; top: parent.top; bottom: splitViewInput.top}
        anchors.leftMargin: 30
        anchors.rightMargin: 30
        anchors.topMargin: 41
        contentWidth: col2.width
        contentHeight: col2.height
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: col2
            anchors.top: parent.top
            spacing: 25
            width: settings.width - 60

            StepsCheckBox {
                id: savePower
                text: qsTr("Conserve power")
                checked: prefs.savePower
            }

            StepsCheckBox {
                id: audioFeedback
                text: qsTr("Sound effects")
                checked: !prefs.muted
            }

            StepsCheckBox {
                id: showExit
                text: qsTr("Show Exit icon")
                checked: prefs.showExit
            }

            StepsLabel {text: qsTr("Sensitivity:")}

            StepsSlider {
                id: sensitivitySlider
                width: parent.width - 15
                stepSize: 10
                valueIndicatorVisible: true
                minimumValue: 10
                maximumValue: 190
                value: counter.sensitivity
            }

            StepsLabel {text: qsTr("Save to Google Docs:")}

            StepsCheckBox {
                text: qsTr("Enable saving")
                id: enableSharing
                checked: gft.enabled
                enabled: gft.linked
            }

            StepsButton {
                text: gft.linked? qsTr("Logout from Google"): qsTr("Login to Google")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    if (gft.linked) {
                        confirmLogoutDialog.open()
                    } else {
                        spinner.running = true
                        gft.link()
                    }
                }
            }

            StepsLabel {
                text: qsTr("Rename activity \"Custom 1\":")
                visible: platform.osName !== "symbian"
            }

            StepsTextField {
                id: custom1Text
                visible: platform.osName !== "symbian"
                width: parent.width
                text: main.activityNames[2]
            }

            StepsLabel {
                text: qsTr("Rename activity \"Custom 2\":")
                visible: platform.osName !== "symbian"
            }

            StepsTextField {
                id: custom2Text
                visible: platform.osName !== "symbian"
                width: parent.width
                text: main.activityNames[3]
            }
        }
    }

//    Item {
//        id: splitViewInput
//        anchors {bottom: parent.bottom; left: parent.left; right: parent.right}
//        Behavior on height {PropertyAnimation {duration: 200}}
//        states: [
//            State {
//                name: "Visible"; when: inputContext.visible
//                PropertyChanges {target: splitViewInput; height: inputContext.height}
//                PropertyChanges {
//                    target: flickable
//                    interactive:false
//                }
//            },
//            State {
//                name: "Hidden"; when: !inputContext.visible
//                PropertyChanges {target: splitViewInput; height: 0} // mainPage.pageStack.toolbar}
//                PropertyChanges {
//                    target: flickable
//                    interactive:true
//                }
//            }
//        ]
//        onStateChanged: {
//            if(state=="Visible") {
//                console.debug("input now visible, height is " + inputContext.height);
//                // flickable.contentY = flickable.lasty;
//            } else if(state=="Hidden") {
//                console.debug("input now hidden");
//            }
//        }
//    }

    StepsYesNoDialog {
        id: confirmLogoutDialog
        titleText: qsTr("Are you sure to log out?")
        onDialogAccepted: {
            gft.unlink()
        }
    }

    StepsSpinner {
        id: spinner
    }

    LoginBrowser {
        id: loginBrowser
    }

    StepsBanner {
        id: linkInfo
        text: gft.linked? qsTr("Logged in to Google Docs"): qsTr("Logged out from Google Docs")
    }

    onBack: {
        main.pageStack.pop()
        prefs.muted = !audioFeedback.checked
        counter.setSensitivity(sensitivitySlider.value)
        prefs.sensitivity = counter.sensitivity
        gft.enabled = enableSharing.checked
        main.activityNames = [main.activityNames[0], main.activityNames[1], custom1Text.text, custom2Text.text]
        prefs.showExit = showExit.checked
        prefs.savePower = savePower.checked
    }

    function openBrowser(url) {
        spinner.running = false
        main.pageStack.push(loginBrowser)
        loginBrowser.openUrl(url)
    }

    Component.onCompleted: {
        gft.openBrowser.connect(openBrowser);
        gft.linkedChanged.connect(linkInfo.show)
        if (platform.osName !== "symbian") {
            showExit.height = 0
            showExit.visible = false
        }
    }
}
