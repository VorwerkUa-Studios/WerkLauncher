import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Item {
    required property var instanceData
    required property var controller
    required property bool bright
    required property string accentColor
    required property int iconVersion
    required property bool selected

    signal selectRequested(int instanceIndex, string instanceId)

    width: 110
    height: 120

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 4
        radius: 8
        color: selected
               ? Qt.rgba(Qt.color(accentColor).r, Qt.color(accentColor).g, Qt.color(accentColor).b, 0.15)
               : (cardHover.hovered ? Qt.rgba(bright ? 0 : 1, bright ? 0 : 1, bright ? 0 : 1, bright ? 0.05 : 0.08) : "transparent")
        border.color: selected
                      ? accentColor
                      : (cardHover.hovered ? (bright ? "#d1d1d6" : "#333333") : "transparent")
        border.width: selected ? 2 : 1

        Behavior on color { ColorAnimation { duration: 100 } }
        Behavior on border.color { ColorAnimation { duration: 100 } }

        HoverHandler {
            id: cardHover
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 4

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignHCenter

                Image {
                    anchors.centerIn: parent
                    source: instanceData.iconKey
                           ? "image://themedicon/" + instanceData.iconKey + "?v=" + iconVersion
                           : ""
                    width: 48
                    height: 48
                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: 48
                    sourceSize.height: 48
                }
            }

            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: instanceData.display || ""
                color: bright ? "#1a1a1b" : "#cccccc"
                font.pixelSize: 11
                font.family: "Segoe UI"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            cursorShape: Qt.PointingHandCursor

            onClicked: (mouse) => {
                selectRequested(instanceData.modelIndex, instanceData.instanceId)
                if (mouse.button === Qt.RightButton && controller) {
                    controller.showInstanceContextMenuFromQml()
                }
            }

            onDoubleClicked: {
                selectRequested(instanceData.modelIndex, instanceData.instanceId)
                if (controller) {
                    controller.on_actionLaunchInstance_triggered()
                }
            }
        }

        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 6
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            opacity: cardHover.hovered ? 1 : 0
            visible: opacity > 0
            z: 10

            Behavior on opacity { NumberAnimation { duration: 150 } }

            Button {
                id: playButton
                width: 30
                height: 30
                background: Rectangle {
                    color: playButton.hovered ? accentColor : (bright ? "#ffffff" : Qt.rgba(0, 0, 0, 0.3))
                    border.color: playButton.hovered ? accentColor : (bright ? "#d1d1d6" : Qt.rgba(1, 1, 1, 0.1))
                    radius: 15
                }
                contentItem: Item {
                    Image {
                        id: playImage
                        anchors.centerIn: parent
                        source: controller ? controller.getThemedIconUrl("status-running") + "?v=" + iconVersion : ""
                        width: 14
                        height: 14
                        visible: false
                    }
                    ColorOverlay {
                        anchors.fill: playImage
                        source: playImage
                        color: bright ? (playButton.hovered ? "#fff" : "#555") : (playButton.hovered ? "#fff" : "#ccc")
                    }
                }
                onClicked: {
                    selectRequested(instanceData.modelIndex, instanceData.instanceId)
                    if (controller) controller.on_actionLaunchInstance_triggered()
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTranslate("MainWindow", "Launch")
            }

            Button {
                id: settingsButton
                width: 30
                height: 30
                background: Rectangle {
                    color: settingsButton.hovered ? accentColor : (bright ? "#ffffff" : Qt.rgba(0, 0, 0, 0.3))
                    border.color: settingsButton.hovered ? accentColor : (bright ? "#d1d1d6" : Qt.rgba(1, 1, 1, 0.1))
                    radius: 15
                }
                contentItem: Item {
                    Image {
                        id: settingsImage
                        anchors.centerIn: parent
                        source: controller ? controller.getThemedIconUrl("settings") + "?v=" + iconVersion : ""
                        width: 16
                        height: 16
                        visible: false
                    }
                    ColorOverlay {
                        anchors.fill: settingsImage
                        source: settingsImage
                        color: bright ? (settingsButton.hovered ? "#fff" : "#555") : (settingsButton.hovered ? "#fff" : "#ccc")
                    }
                }
                onClicked: {
                    selectRequested(instanceData.modelIndex, instanceData.instanceId)
                    if (controller) controller.on_actionEditInstance_triggered()
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTranslate("MainWindow", "Edit Instance")
            }
        }
    }
}
