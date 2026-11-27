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

        ScrollView {
            anchors.fill: parent
            anchors.margins: 18
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ColumnLayout {
                width: parent.width
                spacing: 14

                // Shortcut path
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        text: qsTranslate("CreateShortcutDialog", "Shortcut path:")
                        color: palette.windowText
                        font.bold: true
                        font.pixelSize: 13
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        TextField {
                            Layout.fillWidth: true
                            text: root.dialogController.shortcutPath
                            onTextChanged: root.dialogController.shortcutPath = text
                        }

                        Button {
                            text: qsTranslate("CreateShortcutDialog", "Browse...")
                            onClicked: root.dialogController.browseShortcutPath()
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.4)
                }

                // Join world on launch
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    CheckBox {
                        id: joinWorldCheck
                        text: qsTranslate("CreateShortcutDialog", "Join world on launch:")
                        checked: root.dialogController.joinWorld
                        onToggled: root.dialogController.joinWorld = checked
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 24
                        spacing: 8
                        enabled: root.dialogController.joinWorld
                        opacity: enabled ? 1 : 0.55

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            RadioButton {
                                text: qsTranslate("CreateShortcutDialog", "Server:")
                                checked: root.dialogController.joinServerMode
                                onToggled: root.dialogController.joinServerMode = true
                            }

                            TextField {
                                Layout.fillWidth: true
                                enabled: root.dialogController.joinServerMode
                                placeholderText: "mc.example.com"
                                text: root.dialogController.serverAddress
                                onTextChanged: root.dialogController.serverAddress = text
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            visible: root.dialogController.instanceSupportsQuickPlay

                            RadioButton {
                                text: qsTranslate("CreateShortcutDialog", "World:")
                                checked: !root.dialogController.joinServerMode
                                onToggled: root.dialogController.joinServerMode = false
                            }

                            ComboBox {
                                Layout.fillWidth: true
                                enabled: !root.dialogController.joinServerMode
                                model: root.dialogController.singleplayerWorlds
                                onActivated: function(index) {
                                    root.dialogController.selectedSingleplayerWorld = model[index];
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.4)
                }

                // Profile
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    CheckBox {
                        text: qsTranslate("CreateShortcutDialog", "Use specific profile:")
                        checked: root.dialogController.useProfile
                        onToggled: root.dialogController.useProfile = checked
                    }

                    ComboBox {
                        Layout.fillWidth: true
                        enabled: root.dialogController.useProfile
                        opacity: enabled ? 1 : 0.55
                        model: root.dialogController.availableProfiles
                        currentIndex: {
                            for (var i = 0; i < model.length; i++) {
                                if (model[i] === root.dialogController.selectedProfile) return i;
                            }
                            return 0;
                        }
                        onActivated: function(index) {
                            root.dialogController.selectedProfile = model[index];
                        }
                    }
                }

                // Offline mode
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    CheckBox {
                        text: qsTranslate("CreateShortcutDialog", "Launch in offline mode")
                        checked: root.dialogController.launchOffline
                        onToggled: root.dialogController.launchOffline = checked
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 24
                        spacing: 8
                        enabled: root.dialogController.launchOffline
                        opacity: enabled ? 1 : 0.55

                        CheckBox {
                            text: qsTranslate("CreateShortcutDialog", "Set username:")
                            checked: root.dialogController.setOfflineUsername
                            onToggled: root.dialogController.setOfflineUsername = checked
                        }

                        TextField {
                            Layout.fillWidth: true
                            enabled: root.dialogController.setOfflineUsername
                            placeholderText: "Player"
                            text: root.dialogController.offlineUsername
                            onTextChanged: root.dialogController.offlineUsername = text
                        }
                    }
                }

                // Script checkbox
                CheckBox {
                    visible: root.dialogController.createScriptEnabled
                    Layout.fillWidth: true
                    text: qsTranslate("CreateShortcutDialog", "Create script instead of link")
                    checked: root.dialogController.createScript
                    onToggled: root.dialogController.createScript = checked
                }

                Item { Layout.fillHeight: true; Layout.preferredHeight: 8 }

                // Buttons
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Item { Layout.fillWidth: true }

                    Button {
                        text: qsTranslate("CreateShortcutDialog", "Cancel")
                        onClicked: root.dialogController.cancelAction()
                    }

                    Button {
                        text: qsTranslate("CreateShortcutDialog", "Create")
                        highlighted: true
                        enabled: root.dialogController.canCreate
                        onClicked: root.dialogController.createShortcutAction()
                    }
                }
            }
        }
    }
}
