import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var dialogController

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    Rectangle {
        anchors.fill: parent
        color: palette.window

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                text: qsTranslate("IconPickerDialog", "Select Icon")
                color: palette.windowText
                font.pixelSize: 16
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: palette.base
                border.color: palette.mid
                clip: true

                GridView {
                    id: iconGrid
                    anchors.fill: parent
                    anchors.margins: 10
                    cellWidth: 60
                    cellHeight: 60
                    clip: true
                    model: root.dialogController.iconKeys
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    delegate: Rectangle {
                        width: 52
                        height: 52
                        radius: 8
                        color: root.dialogController.selectedIconKey === modelData ? Qt.rgba(palette.highlight.r, palette.highlight.g, palette.highlight.b, 0.25) : (gridMouse.containsMouse ? Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.2) : "transparent")
                        border.color: root.dialogController.selectedIconKey === modelData ? palette.highlight : "transparent"
                        border.width: 2

                        MouseArea {
                            id: gridMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: root.dialogController.selectedIconKey = modelData
                            onDoubleClicked: root.dialogController.chooseIcon(modelData)
                        }

                        Image {
                            anchors.centerIn: parent
                            width: 36
                            height: 36
                            source: "image://themedicon/" + modelData
                            fillMode: Image.PreserveAspectFit
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Button {
                    text: qsTranslate("IconPickerDialog", "Add Icon")
                    onClicked: root.dialogController.addNewIcon()
                }

                Button {
                    text: qsTranslate("IconPickerDialog", "Remove Icon")
                    enabled: root.dialogController.canRemove
                    onClicked: root.dialogController.removeSelectedIcon()
                }

                Button {
                    text: qsTranslate("IconPickerDialog", "Open Folder")
                    onClicked: root.dialogController.openFolder()
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTranslate("IconPickerDialog", "Cancel")
                    onClicked: root.dialogController.rejectDialog()
                }

                Button {
                    text: qsTranslate("IconPickerDialog", "OK")
                    highlighted: true
                    onClicked: root.dialogController.acceptDialog()
                }
            }
        }
    }
}
