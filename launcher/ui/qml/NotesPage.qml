import QtQuick
import QtQuick.Controls

Item {
    id: root
    required property var pageController

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    TextArea {
        anchors.fill: parent
        text: root.pageController.noteText
        onTextChanged: {
            if (text !== root.pageController.noteText)
                root.pageController.noteText = text
        }
        textFormat: TextEdit.PlainText
        wrapMode: TextEdit.Wrap
        selectByMouse: true
        color: palette.text
        background: Rectangle {
            color: palette.base
        }
    }
}
