import QtQuick 1.1
import "meego"

StepsDialog {
    id: googleLogin

    title: "Login to Google"
    acceptText: "Login"
    rejectText: "Cancel"

    content: Flickable {
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.rightMargin: 30
        anchors.topMargin: 41
        contentWidth: col2.width
        contentHeight: col2.height
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: col2
            anchors.top: parent.top
            spacing: 31
            width: googleLogin.width - 60

            StepsLabel {
                width: parent.width
                text: "E-mail address:"
                // inputMethodHints: 0x22
            }
            StepsTextField {
                width: parent.width
                id: email
            }
            StepsLabel {
                text: "Password:"
            }
            StepsTextField {
                width: parent.width
                id: password
                echoMode: TextInput.PasswordEchoOnEdit
            }
        }
    }

    onDialogAccepted: {
        console.log("* GoogleLogin.onDialogAccepted: '" + email.text + "', '" + password.text + "'")
        googleDocs.link(email.text, password.text)
    }
}
