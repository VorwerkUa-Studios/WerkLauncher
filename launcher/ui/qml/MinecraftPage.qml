import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var pageController

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
            id: mainContent
            x: (scrollView.availableWidth - width) / 2
            width: Math.max(0, Math.min(860, scrollView.availableWidth - 40))
            spacing: 16

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: qsTranslate("MinecraftPage", "Minecraft")
                    color: palette.windowText
                    font.pixelSize: 22
                    font.bold: true
                }
            }

            SettingCard {
                Layout.fillWidth: true
                title: qsTranslate("MinecraftPage", "Window Size")

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("MinecraftPage", "Start Minecraft maximized?")
                    checked: root.pageController.launchMaximized
                    onToggled: root.pageController.launchMaximized = checked
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 24
                    enabled: !root.pageController.launchMaximized
                    opacity: enabled ? 1 : 0.55

                    RowLayout {
                        spacing: 8
                        Text {
                            text: qsTranslate("MinecraftPage", "Window width:")
                            color: palette.windowText
                        }
                        SpinBox {
                            from: 1
                            to: 65536
                            stepSize: 10
                            editable: true
                            value: root.pageController.windowWidth
                            onValueModified: root.pageController.windowWidth = value
                        }
                    }

                    RowLayout {
                        spacing: 8
                        Text {
                            text: qsTranslate("MinecraftPage", "Window height:")
                            color: palette.windowText
                        }
                        SpinBox {
                            from: 1
                            to: 65536
                            stepSize: 10
                            editable: true
                            value: root.pageController.windowHeight
                            onValueModified: root.pageController.windowHeight = value
                        }
                    }
                }
            }

            SettingCard {
                Layout.fillWidth: true
                title: qsTranslate("MinecraftPage", "Native library workarounds")

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("MinecraftPage", "Use system installation of GLFW")
                    checked: root.pageController.useNativeGLFW
                    onToggled: root.pageController.useNativeGLFW = checked
                }

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("MinecraftPage", "Use system installation of OpenAL")
                    checked: root.pageController.useNativeOpenAL
                    onToggled: root.pageController.useNativeOpenAL = checked
                }
            }

            SettingCard {
                Layout.fillWidth: true
                title: qsTranslate("MinecraftPage", "Game time")

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("MinecraftPage", "Show time spent playing instances")
                    checked: root.pageController.showGameTime
                    onToggled: root.pageController.showGameTime = checked
                }

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("MinecraftPage", "Show time spent playing across all instances")
                    checked: root.pageController.showGlobalGameTime
                    onToggled: root.pageController.showGlobalGameTime = checked
                }

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("MinecraftPage", "Record time spent playing instances")
                    checked: root.pageController.recordGameTime
                    onToggled: root.pageController.recordGameTime = checked
                }

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("MinecraftPage", "Show time spent playing in hours only")
                    checked: root.pageController.showGameTimeHours
                    onToggled: root.pageController.showGameTimeHours = checked
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
