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
                text: qsTranslate("CopyInstanceDialog", "Copy Instance")
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

                    // Icon + Name
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Button {
                            id: iconBtn
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            onClicked: root.dialogController.pickIcon()

                            Image {
                                anchors.centerIn: parent
                                width: 32
                                height: 32
                                source: "image://themedicon/" + (root.dialogController.iconKey || "default")
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: qsTranslate("CopyInstanceDialog", "Name:")
                                color: palette.windowText
                                font.pixelSize: 12
                                opacity: 0.8
                            }

                            TextField {
                                Layout.fillWidth: true
                                text: root.dialogController.instName
                                placeholderText: qsTranslate("CopyInstanceDialog", "Instance name")
                                onTextChanged: root.dialogController.instName = text
                            }
                        }
                    }

                    // Group
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: qsTranslate("CopyInstanceDialog", "Group:")
                            color: palette.windowText
                            font.pixelSize: 12
                            opacity: 0.8
                        }

                        ComboBox {
                            Layout.fillWidth: true
                            editable: true
                            model: root.dialogController.availableGroups
                            currentIndex: {
                                var g = root.dialogController.instGroup;
                                for (var i = 0; i < model.length; i++) {
                                    if (model[i] === g) return i;
                                }
                                return 0;
                            }
                            onEditTextChanged: root.dialogController.instGroup = editText
                            onActivated: function(index) {
                                root.dialogController.instGroup = model[index];
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.4)
                    }

                    // Options
                    CheckBox {
                        Layout.fillWidth: true
                        text: qsTranslate("CopyInstanceDialog", "Copy saves?")
                        checked: root.dialogController.shouldCopySaves
                        onToggled: root.dialogController.shouldCopySaves = checked
                    }

                    CheckBox {
                        Layout.fillWidth: true
                        text: qsTranslate("CopyInstanceDialog", "Keep playtime?")
                        checked: root.dialogController.shouldKeepPlaytime
                        onToggled: root.dialogController.shouldKeepPlaytime = checked
                    }

                    Item { Layout.fillHeight: true }
                }
            }

            // Footer
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTranslate("CopyInstanceDialog", "Cancel")
                    onClicked: root.dialogController.rejectDialog()
                }

                Button {
                    text: qsTranslate("CopyInstanceDialog", "Copy")
                    highlighted: true
                    enabled: root.dialogController.canAccept
                    onClicked: root.dialogController.acceptDialog()
                }
            }
        }
    }
}
