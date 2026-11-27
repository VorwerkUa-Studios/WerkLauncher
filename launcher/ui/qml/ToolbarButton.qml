import QtQuick
import QtQuick.Controls

Item {
    id: control

    implicitWidth: 42
    implicitHeight: 42
    activeFocusOnTab: true

    property bool bright: false
    property string accentColor: "#ffae00"
    property color accentForeground: "#000000"
    property int iconVersion: 0
    property string iconSource: ""
    property string tooltip: ""

    signal clicked()

    Accessible.name: tooltip
    Accessible.role: Accessible.Button

    Keys.onReturnPressed: clicked()
    Keys.onSpacePressed: clicked()

    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        radius: 12
        color: control.activeFocus || buttonMouseArea.containsMouse
               ? Qt.rgba(Qt.color(control.accentColor).r,
                         Qt.color(control.accentColor).g,
                         Qt.color(control.accentColor).b, control.bright ? 0.12 : 0.18)
               : (control.bright ? "#ffffff" : "#252629")
        border.color: control.activeFocus || buttonMouseArea.containsMouse
                      ? control.accentColor
                      : (control.bright ? "#e5e5ea" : "#34363a")
        border.width: 1

        Behavior on color { ColorAnimation { duration: 120 } }
        Behavior on border.color { ColorAnimation { duration: 120 } }

        Image {
            anchors.centerIn: parent
            source: {
                if (!control.iconSource) {
                    return ""
                }
                return control.iconSource.indexOf("data:") === 0
                        ? control.iconSource
                        : control.iconSource + "?v=" + control.iconVersion
            }
            width: control.iconSize
            height: control.iconSize
            fillMode: Image.PreserveAspectFit
            opacity: control.enabled ? 1 : 0.5
        }
    }

    property int iconSize: 24

    ToolTip {
        id: buttonTooltip
        text: control.tooltip
        visible: buttonMouseArea.containsMouse && control.tooltip !== ""
        delay: 500

        contentItem: Text {
            text: buttonTooltip.text
            color: control.accentForeground
            font.pixelSize: 12
            font.family: "Segoe UI"
        }

        background: Rectangle {
            color: control.accentColor
            radius: 4
        }
    }

    MouseArea {
        id: buttonMouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: control.clicked()
    }
}
