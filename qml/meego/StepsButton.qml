import QtQuick 1.0
import com.nokia.meego 1.0
import com.nokia.extras 1.1

Button {
    property bool negative: false
    style: ButtonStyle {background: negative? "image://theme/meegotouch-button-negative-background": "image://theme/meegotouch-button" + __invertedString + "-background"}
}
