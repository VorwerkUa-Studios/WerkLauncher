import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var settingsController

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            id: settingsContent
            x: (scrollView.availableWidth - width) / 2
            width: Math.max(0, Math.min(860, scrollView.availableWidth - 40))
            spacing: 16

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: qsTranslate("InstanceSettingsPage", "Instance settings")
                    color: palette.windowText
                    font.pixelSize: 22
                    font.bold: true
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTranslate("InstanceSettingsPage",
                                      "Override selected global settings for this instance.")
                    color: palette.windowText
                    opacity: 0.72
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }

            SettingCard {
                Layout.fillWidth: true
                title: qsTranslate("InstanceSettingsPage", "Game window")

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("InstanceSettingsPage",
                                      "Override global game window settings")
                    checked: root.settingsController.windowOverride
                    onToggled: root.settingsController.windowOverride = checked
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 28
                    spacing: 12
                    enabled: root.settingsController.windowOverride
                    opacity: enabled ? 1 : 0.55

                    CheckBox {
                        text: qsTranslate("InstanceSettingsPage",
                                          "Start Minecraft maximized")
                        checked: root.settingsController.launchMaximized
                        onToggled: root.settingsController.launchMaximized = checked
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        Text {
                            Layout.fillWidth: true
                            text: qsTranslate("InstanceSettingsPage", "Width")
                            color: palette.windowText
                        }

                        SpinBox {
                            from: 1
                            to: 65536
                            editable: true
                            value: root.settingsController.windowWidth
                            onValueModified: root.settingsController.windowWidth = value
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        Text {
                            Layout.fillWidth: true
                            text: qsTranslate("InstanceSettingsPage", "Height")
                            color: palette.windowText
                        }

                        SpinBox {
                            from: 1
                            to: 65536
                            editable: true
                            value: root.settingsController.windowHeight
                            onValueModified: root.settingsController.windowHeight = value
                        }
                    }
                }
            }

            SettingCard {
                Layout.fillWidth: true
                title: qsTranslate("InstanceSettingsPage", "Console")

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("InstanceSettingsPage",
                                      "Override global console settings")
                    checked: root.settingsController.consoleOverride
                    onToggled: root.settingsController.consoleOverride = checked
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 28
                    spacing: 8
                    enabled: root.settingsController.consoleOverride
                    opacity: enabled ? 1 : 0.55

                    CheckBox {
                        Layout.fillWidth: true
                        text: qsTranslate("InstanceSettingsPage",
                                          "Show console while the game is running")
                        checked: root.settingsController.showConsole
                        onToggled: root.settingsController.showConsole = checked
                    }

                    CheckBox {
                        Layout.fillWidth: true
                        text: qsTranslate("InstanceSettingsPage",
                                          "Automatically close console when the game quits")
                        checked: root.settingsController.autoCloseConsole
                        onToggled: root.settingsController.autoCloseConsole = checked
                    }

                    CheckBox {
                        Layout.fillWidth: true
                        text: qsTranslate("InstanceSettingsPage",
                                          "Show console when the game crashes")
                        checked: root.settingsController.showConsoleOnError
                        onToggled: root.settingsController.showConsoleOnError = checked
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
            }
        }
    }

    component SettingCard: Rectangle {
        id: card
        required property string title
        default property alias contentData: contents.data

        implicitHeight: contents.implicitHeight + 32
        radius: 10
        color: palette.base
        border.color: palette.mid

        ColumnLayout {
            id: contents
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: card.title
                color: palette.windowText
                font.pixelSize: 16
                font.bold: true
            }
        }
    }
}
