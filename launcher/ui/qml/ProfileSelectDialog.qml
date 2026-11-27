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
            anchors.margins: 18
            spacing: 14

            Text {
                visible: !!root.dialogController.message
                Layout.fillWidth: true
                text: root.dialogController.message
                color: palette.windowText
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: palette.base
                border.color: palette.mid
                clip: true

                ListView {
                    id: listView
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6
                    clip: true
                    model: root.dialogController.accountsList

                    delegate: Rectangle {
                        width: listView.width
                        implicitHeight: 46
                        radius: 8
                        color: root.dialogController.selectedIndex === index ? palette.highlight : (itemMouse.containsMouse ? Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.2) : "transparent")

                        MouseArea {
                            id: itemMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: root.dialogController.selectedIndex = index
                            onDoubleClicked: {
                                root.dialogController.selectedIndex = index
                                root.dialogController.acceptDialog()
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 12

                            Rectangle {
                                width: 28
                                height: 28
                                radius: 4
                                color: Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.3)
                                Text {
                                    anchors.centerIn: parent
                                    text: "👤"
                                    font.pixelSize: 16
                                }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: modelData.name + (modelData.inUse ? " (" + qsTranslate("ProfileSelectDialog", "in use") + ")" : "")
                                color: root.dialogController.selectedIndex === index ? palette.highlightedText : palette.windowText
                                font.pixelSize: 13
                                font.bold: root.dialogController.selectedIndex === index
                            }
                        }
                    }
                }
            }

            CheckBox {
                visible: root.dialogController.showGlobalDefault
                Layout.fillWidth: true
                text: qsTranslate("ProfileSelectDialog", "Remember selection and use as default")
                checked: root.dialogController.useAsGlobalDefault
                onToggled: root.dialogController.useAsGlobalDefault = checked
            }

            CheckBox {
                visible: root.dialogController.showInstanceDefault
                Layout.fillWidth: true
                text: qsTranslate("ProfileSelectDialog", "Use as default for this instance")
                checked: root.dialogController.useAsInstDefault
                onToggled: root.dialogController.useAsInstDefault = checked
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTranslate("ProfileSelectDialog", "Cancel")
                    onClicked: root.dialogController.rejectDialog()
                }

                Button {
                    text: qsTranslate("ProfileSelectDialog", "OK")
                    highlighted: true
                    enabled: root.dialogController.canAccept
                    onClicked: root.dialogController.acceptDialog()
                }
            }
        }
    }
}
