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
            anchors.margins: 20
            spacing: 16

            Text {
                text: root.dialogController.dialogTitle
                color: palette.windowText
                font.pixelSize: 18
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: palette.base
                border.color: palette.mid

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 8

                    Text {
                        text: root.dialogController.label
                        color: palette.windowText
                        font.pixelSize: 13
                    }

                    TextField {
                        id: inputField
                        Layout.fillWidth: true
                        text: root.dialogController.textValue
                        onTextChanged: root.dialogController.textValue = text
                        Component.onCompleted: {
                            forceActiveFocus();
                            selectAll();
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }

            // Buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTranslate("GroupInputDialog", "Cancel")
                    onClicked: root.dialogController.rejectDialog()
                }

                Button {
                    text: qsTranslate("GroupInputDialog", "OK")
                    highlighted: true
                    enabled: root.dialogController.canAccept
                    onClicked: root.dialogController.acceptDialog()
                }
            }
        }
    }
}
