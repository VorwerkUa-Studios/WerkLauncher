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
                text: qsTranslate("NewComponentDialog", "New Component")
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
                    spacing: 14

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: qsTranslate("NewComponentDialog", "Name:")
                            color: palette.windowText
                            font.pixelSize: 12
                            opacity: 0.8
                        }

                        TextField {
                            id: nameField
                            Layout.fillWidth: true
                            text: root.dialogController.name
                            placeholderText: qsTranslate("NewComponentDialog", "Component Name")
                            onTextChanged: root.dialogController.name = text
                            Component.onCompleted: forceActiveFocus()
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: qsTranslate("NewComponentDialog", "UID:")
                            color: palette.windowText
                            font.pixelSize: 12
                            opacity: 0.8
                        }

                        TextField {
                            id: uidField
                            Layout.fillWidth: true
                            text: root.dialogController.uid
                            placeholderText: root.dialogController.placeholderUid
                            onTextChanged: root.dialogController.uid = text
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
                    text: qsTranslate("NewComponentDialog", "Cancel")
                    onClicked: root.dialogController.rejectDialog()
                }

                Button {
                    text: qsTranslate("NewComponentDialog", "OK")
                    highlighted: true
                    enabled: root.dialogController.canAccept
                    onClicked: root.dialogController.acceptDialog()
                }
            }
        }
    }
}
