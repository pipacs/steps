import QtQuick 1.1
import com.nokia.symbian 1.1

CommonDialog {
    property string title
    property string acceptText
    property string rejectText
    signal dialogClosed
    signal dialogAccepted
    signal dialogRejected

    titleText: title
    buttonTexts: [acceptText, rejectText]
    anchors.fill: parent

    onStatusChanged: {
        if (status === DialogStatus.Closed) {
            console.log("* StepsDialog.dialogClosed")
            dialogClosed()
        }
    }

    onButtonClicked: {
        if (index === 0) {
            console.log("* StepsDialog.dialogAccepted")
            dialogAccepted()
        } else {
            console.log("* StepsDialog.dialogRejected")
            dialogRejected()
        }
    }
}
