import QtQuick 1.1
import com.nokia.symbian 1.1

QueryDialog {
    property string title
    signal dialogClosed
    signal dialogAccepted
    signal dialogRejected

    titleText: "Steps"
    message: title
    buttonTexts: ["Yes", "No"]
    // icon: "/images/steps.png"

    onStatusChanged: {
        if (status === DialogStatus.Closed) {
            dialogClosed()
        }
    }

    onButtonClicked: {
        console.log("* StepsYesNoDialog.onButtonClicked " + index)
        if (index === 0) {
            dialogAccepted()
        } else {
            dialogRejected()
        }
    }
}
