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
                    text: qsTranslate("LauncherPage", "Launcher")
                    color: palette.windowText
                    font.pixelSize: 22
                    font.bold: true
                }
            }

            // Appearance Section
            SettingCard {
                Layout.fillWidth: true
                title: qsTranslate("LauncherPage", "Appearance")

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Text {
                        Layout.fillWidth: true
                        text: qsTranslate("LauncherPage", "Application theme")
                        color: palette.windowText
                    }

                    ComboBox {
                        id: themeCombo
                        Layout.preferredWidth: 200
                        model: root.pageController.availableThemes
                        textRole: "name"
                        currentIndex: {
                            for (var i = 0; i < count; i++) {
                                if (model[i].id === root.pageController.applicationTheme) return i;
                            }
                            return 0;
                        }
                        onActivated: function(index) {
                            root.pageController.applicationTheme = model[index].id;
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Text {
                        Layout.fillWidth: true
                        text: qsTranslate("LauncherPage", "Accent color")
                        color: palette.windowText
                    }

                    Button {
                        text: qsTranslate("LauncherPage", "Select Color")
                        onClicked: root.pageController.pickAccentColor()

                        Rectangle {
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            width: 14
                            height: 14
                            radius: 3
                            color: root.pageController.customAccentColor || palette.highlight
                            border.color: palette.mid
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Text {
                        Layout.fillWidth: true
                        text: qsTranslate("LauncherPage", "Icon theme")
                        color: palette.windowText
                    }

                    ComboBox {
                        Layout.preferredWidth: 200
                        model: [
                            { text: qsTranslate("LauncherPage", "Flat"), value: "flat" },
                            { text: qsTranslate("LauncherPage", "Colored"), value: "colored" },
                            { text: qsTranslate("LauncherPage", "White"), value: "white" }
                        ]
                        textRole: "text"
                        currentIndex: {
                            var v = root.pageController.iconTheme;
                            if (v === "colored") return 1;
                            if (v === "white") return 2;
                            return 0;
                        }
                        onActivated: function(index) {
                            root.pageController.iconTheme = model[index].value;
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.4)
                }

                Text {
                    text: qsTranslate("LauncherPage", "Background Image")
                    color: palette.windowText
                    font.bold: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Text {
                        Layout.fillWidth: true
                        text: root.pageController.customBackgroundImage ? root.pageController.customBackgroundImage.split('/').pop().split('\\').pop() : qsTranslate("LauncherPage", "None")
                        color: palette.windowText
                        opacity: 0.75
                        elide: Text.ElideMiddle
                    }

                    Button {
                        text: qsTranslate("LauncherPage", "Choose...")
                        onClicked: root.pageController.chooseBackgroundImage()
                    }

                    Button {
                        text: qsTranslate("LauncherPage", "Clear")
                        enabled: !!root.pageController.customBackgroundImage
                        onClicked: root.pageController.clearBackgroundImage()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16
                    enabled: !!root.pageController.customBackgroundImage
                    opacity: enabled ? 1 : 0.55

                    Text {
                        text: qsTranslate("LauncherPage", "Background opacity")
                        color: palette.windowText
                    }

                    Slider {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        stepSize: 1
                        value: root.pageController.customBackgroundOpacity
                        onMoved: root.pageController.customBackgroundOpacity = Math.round(value)
                    }

                    Text {
                        text: root.pageController.customBackgroundOpacity + "%"
                        color: palette.windowText
                        Layout.preferredWidth: 40
                    }
                }

                CheckBox {
                    Layout.fillWidth: true
                    enabled: !!root.pageController.customBackgroundImage
                    opacity: enabled ? 1 : 0.55
                    text: qsTranslate("LauncherPage", "Blur background image")
                    checked: root.pageController.customBackgroundBlur
                    onToggled: root.pageController.customBackgroundBlur = checked
                }
            }

            // Folders Section
            SettingCard {
                Layout.fillWidth: true
                title: qsTranslate("LauncherPage", "Folders")

                FolderItem {
                    Layout.fillWidth: true
                    label: qsTranslate("LauncherPage", "Instances:")
                    folderPath: root.pageController.instanceDir
                    onBrowse: root.pageController.browseInstanceDir()
                    onOpen: root.pageController.openInstanceDir()
                }

                FolderItem {
                    Layout.fillWidth: true
                    label: qsTranslate("LauncherPage", "Icons:")
                    folderPath: root.pageController.iconsDir
                    onBrowse: root.pageController.browseIconsDir()
                    onOpen: root.pageController.openIconsDir()
                }

                FolderItem {
                    Layout.fillWidth: true
                    label: qsTranslate("LauncherPage", "Skins:")
                    folderPath: root.pageController.skinsDir
                    onBrowse: root.pageController.browseSkinsDir()
                    onOpen: root.pageController.openSkinsDir()
                }
            }

            // General & System Section
            SettingCard {
                Layout.fillWidth: true
                title: qsTranslate("LauncherPage", "General")

                CheckBox {
                    Layout.fillWidth: true
                    visible: root.pageController.updaterEnabled
                    text: qsTranslate("LauncherPage", "Check for updates on start?")
                    checked: root.pageController.autoUpdate
                    onToggled: root.pageController.autoUpdate = checked
                }

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("LauncherPage", "Minimize to tray on start?")
                    checked: root.pageController.minimizeToTray
                    onToggled: root.pageController.minimizeToTray = checked
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Text {
                        Layout.fillWidth: true
                        text: qsTranslate("LauncherPage", "Sort instances by:")
                        color: palette.windowText
                    }

                    ComboBox {
                        Layout.preferredWidth: 200
                        model: [
                            { text: qsTranslate("LauncherPage", "Name"), value: "Name" },
                            { text: qsTranslate("LauncherPage", "Last played"), value: "LastLaunch" }
                        ]
                        textRole: "text"
                        currentIndex: root.pageController.instSortMode === "LastLaunch" ? 1 : 0
                        onActivated: function(index) {
                            root.pageController.instSortMode = model[index].value;
                        }
                    }
                }

                Button {
                    text: qsTranslate("LauncherPage", "Reset hidden notifications")
                    onClicked: root.pageController.resetNotifications()
                }
            }

            // Console Section
            SettingCard {
                Layout.fillWidth: true
                title: qsTranslate("LauncherPage", "Console")

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("LauncherPage", "Show console when game starts")
                    checked: root.pageController.showConsole
                    onToggled: root.pageController.showConsole = checked
                }

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("LauncherPage", "Automatically close console when game quits")
                    checked: root.pageController.autoCloseConsole
                    onToggled: root.pageController.autoCloseConsole = checked
                }

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("LauncherPage", "Show console when game crashes")
                    checked: root.pageController.showConsoleOnError
                    onToggled: root.pageController.showConsoleOnError = checked
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Text {
                        Layout.fillWidth: true
                        text: qsTranslate("LauncherPage", "Console font size:")
                        color: palette.windowText
                    }

                    SpinBox {
                        from: 6
                        to: 36
                        value: root.pageController.consoleFontSize
                        onValueModified: root.pageController.consoleFontSize = value
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Text {
                        Layout.fillWidth: true
                        text: qsTranslate("LauncherPage", "Maximum log lines:")
                        color: palette.windowText
                    }

                    SpinBox {
                        from: 100
                        to: 1000000
                        stepSize: 1000
                        editable: true
                        value: root.pageController.consoleMaxLines
                        onValueModified: root.pageController.consoleMaxLines = value
                    }
                }

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTranslate("LauncherPage", "Stop logging when limit is reached")
                    checked: root.pageController.consoleOverflowStop
                    onToggled: root.pageController.consoleOverflowStop = checked
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

    component FolderItem: ColumnLayout {
        id: folder
        required property string label
        required property string folderPath
        signal browse()
        signal open()

        spacing: 6

        Text {
            text: folder.label
            color: palette.windowText
            font.bold: true
            font.pixelSize: 13
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 34
                radius: 6
                color: Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.15)
                border.color: palette.mid

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    verticalAlignment: Text.AlignVCenter
                    text: folder.folderPath || ""
                    color: palette.windowText
                    elide: Text.ElideMiddle
                    font.pixelSize: 12
                }
            }

            Button {
                text: qsTranslate("LauncherPage", "Browse...")
                onClicked: folder.browse()
            }

            Button {
                text: qsTranslate("LauncherPage", "Open")
                onClicked: folder.open()
            }
        }
    }
}
