import QtQuick
import QtQuick.Controls

Item {
    id: control

    implicitWidth: 44
    implicitHeight: 44
    activeFocusOnTab: true

    property bool bright: false
    property string accentColor: Theme.accent
    property color accentForeground: Theme.textOnAccent
    property int iconVersion: 0
    property string iconSource: ""
    property string tooltip: ""
    property int iconSize: 22

    signal clicked()

    Accessible.name: tooltip
    Accessible.role: Accessible.Button

    Keys.onReturnPressed: clicked()
    Keys.onSpacePressed: clicked()

    scale: buttonMouseArea.pressed ? 0.94 : (buttonMouseArea.containsMouse ? 1.05 : 1.0)
    Behavior on scale {
        NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 3
        radius: Theme.radiusMd
        color: control.activeFocus || buttonMouseArea.containsMouse
               ? Theme.alpha(Theme.accent, control.bright ? 0.12 : 0.18)
               : "transparent"
        border.color: control.activeFocus || buttonMouseArea.containsMouse
                      ? Theme.alpha(Theme.accent, 0.45)
                      : "transparent"
        border.width: 1

        Behavior on color { ColorAnimation { duration: Theme.animFast } }
        Behavior on border.color { ColorAnimation { duration: Theme.animFast } }

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
            opacity: control.enabled ? (buttonMouseArea.containsMouse ? 1.0 : 0.85) : 0.4
            Behavior on opacity { NumberAnimation { duration: Theme.animFast } }
        }
    }

    ToolTip {
        id: buttonTooltip
        text: control.tooltip
        visible: buttonMouseArea.containsMouse && control.tooltip !== ""
        delay: 400

        contentItem: Text {
            text: buttonTooltip.text
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeSmall
            font.family: Theme.fontFamily
        }

        background: Rectangle {
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            border.width: 1
            radius: Theme.radiusSm
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
