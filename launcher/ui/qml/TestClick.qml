import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
    id: mainWindow
    width: 400
    height: 300
    visible: true
    title: "Click Test"
    flags: Qt.Window | Qt.FramelessWindowHint
    color: "#1a1a1b"

    Text {
        anchors.centerIn: parent
        id: statusText
        text: "Click any button!"
        color: "white"
        font.pixelSize: 20
    }

    Button {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        text: "Test Button 1"
        onClicked: statusText.text = "Button 1 CLICKED!"
    }

    Button {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 20
        text: "Close"
        onClicked: Qt.quit()
    }

    Button {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        text: "Test Button 2"
        onClicked: statusText.text = "Button 2 CLICKED!"
    }

    // Test drag
    MouseArea {
        anchors.fill: parent
        z: -1
        property point clickPos: "0,0"
        onPressed: (mouse) => { clickPos = Qt.point(mouse.x, mouse.y) }
        onPositionChanged: (mouse) => {
            if (pressed) {
                mainWindow.x += mouse.x - clickPos.x
                mainWindow.y += mouse.y - clickPos.y
            }
        }
    }
}
