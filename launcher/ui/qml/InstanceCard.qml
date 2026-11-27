import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Item {
    required property var instanceData
    required property var controller
    required property bool bright
    required property string accentColor
    required property color accentTextColor
    required property int iconVersion
    required property bool selected

    signal selectRequested(int instanceIndex, string instanceId)
    signal contextMenuRequested(real x, real y)

    scale: cardHover.hovered ? 1.025 : 1

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 3
        radius: 14
        color: selected
               ? Qt.tint(bright ? "#ffffff" : "#222222",
                         Qt.rgba(Qt.color(accentColor).r, Qt.color(accentColor).g,
                                 Qt.color(accentColor).b, 0.14))
               : (cardHover.hovered
                      ? (bright ? "#ffffff" : "#2b2b2b")
                      : (bright ? "#ffffff" : "#222222"))
        border.color: selected
                      ? accentColor
                      : (cardHover.hovered ? (bright ? "#c9c9c9" : "#505050")
                                           : (bright ? "#e5e5e5" : "#3a3a3a"))
        border.width: selected ? 2 : 1

        Behavior on color { ColorAnimation { duration: 120 } }
        Behavior on border.color { ColorAnimation { duration: 120 } }

        HoverHandler {
            id: cardHover
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            cursorShape: Qt.PointingHandCursor

            onClicked: (mouse) => {
                selectRequested(instanceData.modelIndex, instanceData.instanceId)
                if (mouse.button === Qt.RightButton && controller) {
                    contextMenuRequested(mouse.x, mouse.y)
                }
            }

            onDoubleClicked: {
                selectRequested(instanceData.modelIndex, instanceData.instanceId)
                if (controller) controller.on_actionLaunchInstance_triggered()
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 3

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 74
                Layout.alignment: Qt.AlignHCenter

                Rectangle {
                    anchors.centerIn: parent
                    width: 64
                    height: 64
                    radius: 18
                    color: bright ? "#f2f2f2" : "#2a2a2a"
                    border.color: bright ? "#e5e5e5" : "#3a3a3a"
                    border.width: 1
                }

                Image {
                    id: instanceIcon
                    anchors.centerIn: parent
                    source: instanceData.iconKey === "default" || instanceData.iconKey === "logo"
                            ? "image://themedicon/logo?v=" + iconVersion
                            : (instanceData.iconKey
                               ? "image://themedicon/" + instanceData.iconKey + "?v=" + iconVersion
                               : "")
                    width: 58
                    height: 58
                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: 58
                    sourceSize.height: 58
                }
            }

            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: 38
                text: instanceData.display || ""
                color: bright ? "#202127" : "#e4e6eb"
                font.pixelSize: 15
                font.family: "Segoe UI"
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 34
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                opacity: cardHover.hovered || selected ? 1 : 0
                enabled: cardHover.hovered || selected

                Behavior on opacity { NumberAnimation { duration: 120 } }

                Button {
                    id: playButton
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    width: 30
                    height: 30
                    background: Rectangle {
                        color: playButton.hovered
                               ? accentColor
                               : (bright ? "#f2f2f2" : "#303030")
                        border.color: playButton.hovered
                                      ? accentColor
                                      : (bright ? "#e1e1e1" : "#414141")
                        radius: 11
                    }
                    contentItem: Item {
                        Image {
                            id: playImage
                            anchors.centerIn: parent
                            source: controller ? controller.getThemedIconUrl("status-running") + "?v=" + iconVersion : ""
                            width: 18
                            height: 18
                            visible: false
                        }
                        ColorOverlay {
                            anchors.fill: playImage
                            source: playImage
                            color: accentTextColor
                        }
                    }
                    onClicked: {
                        selectRequested(instanceData.modelIndex, instanceData.instanceId)
                        if (controller) controller.on_actionLaunchInstance_triggered()
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: qsTranslate("MainWindow", "Launch")
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    id: settingsButton
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    width: 30
                    height: 30
                    background: Rectangle {
                        color: bright ? "#f2f2f2" : "#303030"
                        border.color: bright ? "#e1e1e1" : "#414141"
                        radius: 11
                    }
                    contentItem: Item {
                        Image {
                            id: settingsImage
                            anchors.centerIn: parent
                            source: controller ? controller.getThemedIconUrl("settings") + "?v=" + iconVersion : ""
                            width: 18
                            height: 18
                            visible: false
                        }
                        ColorOverlay {
                            anchors.fill: settingsImage
                            source: settingsImage
                            color: bright ? "#555b65" : "#c5cad2"
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
}
