pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar

    implicitWidth: 48
    color: bright ? "#ffffff" : "#1a1a1b"

    property bool bright: launcherController
                          ? launcherController.themeName === "bright"
                            || launcherController.themeName === "system"
                          : false
    property string accentColor: launcherController
                                 ? launcherController.accentColor
                                 : "#4CAF50"
    property color accentForeground: contrastForeground(accentColor)
    property int iconVersion: 0
    property var accountMenuItems: []

    function contrastForeground(colorValue) {
        const color = Qt.color(colorValue)
        function linear(channel) {
            return channel <= 0.04045
                    ? channel / 12.92
                    : Math.pow((channel + 0.055) / 1.055, 2.4)
        }
        const luminance = 0.2126 * linear(color.r)
                        + 0.7152 * linear(color.g)
                        + 0.0722 * linear(color.b)
        return 1.05 / (luminance + 0.05) >= (luminance + 0.05) / 0.05
                ? "#ffffff" : "#000000"
    }

    function openMenu(menu, button) {
        const position = button.mapToItem(sidebar, 0, button.height)
        menu.x = width
        menu.y = Math.max(0, Math.min(position.y, height - menu.implicitHeight))
        menu.open()
    }

    function openAccountsMenu(button) {
        accountMenuItems = launcherController
                           ? launcherController.getAccountMenuItems()
                           : []
        openMenu(accountMenu, button)
    }

    Connections {
        target: launcherController
        function onIsShowingSettingsChanged(isShowing) {
            if (!isShowing) {
                sidebar.iconVersion++
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: sidebar.bright ? "#e5e5ea" : Qt.rgba(1, 1, 1, 0.06)
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: sidebar.bright ? "#e5e5ea" : Qt.rgba(1, 1, 1, 0.06)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 12
        anchors.bottomMargin: 6
        spacing: 16

        Item { Layout.preferredHeight: 34 }

        ToolbarButton {
            Layout.alignment: Qt.AlignHCenter
            bright: sidebar.bright
            accentColor: sidebar.accentColor
            accentForeground: sidebar.accentForeground
            iconVersion: sidebar.iconVersion
            iconSource: launcherController
                        ? launcherController.getThemedIconUrl("new")
                        : ""
            tooltip: qsTranslate("MainWindow", "Add Instance")
            onClicked: launcherController.on_actionAddInstance_triggered()
        }

        ToolbarButton {
            id: foldersButton
            Layout.alignment: Qt.AlignHCenter
            bright: sidebar.bright
            accentColor: sidebar.accentColor
            accentForeground: sidebar.accentForeground
            iconVersion: sidebar.iconVersion
            iconSource: launcherController
                        ? launcherController.getThemedIconUrl("viewfolder")
                        : ""
            tooltip: qsTranslate("MainWindow", "Folders")
            onClicked: sidebar.openMenu(foldersMenu, foldersButton)
        }

        ToolbarButton {
            Layout.alignment: Qt.AlignHCenter
            bright: sidebar.bright
            accentColor: sidebar.accentColor
            accentForeground: sidebar.accentForeground
            iconVersion: sidebar.iconVersion
            iconSource: launcherController
                        ? launcherController.getThemedIconUrl("settings")
                        : ""
            tooltip: qsTranslate("MainWindow", "Settings")
            onClicked: launcherController.on_actionSettings_triggered()
        }

        ToolbarButton {
            id: helpButton
            Layout.alignment: Qt.AlignHCenter
            bright: sidebar.bright
            accentColor: sidebar.accentColor
            accentForeground: sidebar.accentForeground
            iconVersion: sidebar.iconVersion
            iconSource: launcherController
                        ? launcherController.getThemedIconUrl("help")
                        : ""
            tooltip: qsTranslate("MainWindow", "Help")
            onClicked: sidebar.openMenu(helpMenu, helpButton)
        }

        Item { Layout.fillHeight: true }

        ToolbarButton {
            id: accountsButton
            Layout.alignment: Qt.AlignHCenter
            bright: sidebar.bright
            accentColor: sidebar.accentColor
            accentForeground: sidebar.accentForeground
            iconVersion: sidebar.iconVersion
            iconSize: 28
            iconSource: launcherController
                        ? (launcherController.accountIconUrl !== ""
                           ? launcherController.accountIconUrl
                           : launcherController.getThemedIconUrl("accounts"))
                        : ""
            tooltip: qsTranslate("MainWindow", "Manage Accounts")
            onClicked: sidebar.openAccountsMenu(accountsButton)
        }

        Item { Layout.preferredHeight: 8 }
    }

    ToolbarButton {
        visible: launcherController && launcherController.canGoBack
        anchors.top: parent.top
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        bright: sidebar.bright
        accentColor: sidebar.accentColor
        accentForeground: sidebar.accentForeground
        iconVersion: sidebar.iconVersion
        iconSource: launcherController
                    ? launcherController.getThemedIconUrl("back")
                    : ""
        tooltip: qsTranslate("MainWindow", "Back")
        onClicked: launcherController.goBackToInstanceView()
    }

    Menu {
        id: foldersMenu
        popupType: Popup.Window
        width: 250
        delegate: StyledMenuItem { }
        background: Rectangle {
            color: sidebar.bright ? "#ffffff" : "#252526"
            border.color: sidebar.bright ? "#e5e5ea" : "#3a3a3b"
            radius: 8
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "View Instance Folder")
            onTriggered: launcherController.on_actionViewInstanceFolder_triggered()
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "View Central Mods Folder")
            onTriggered: launcherController.on_actionViewCentralModsFolder_triggered()
        }
    }

    Menu {
        id: helpMenu
        popupType: Popup.Window
        width: 240
        delegate: StyledMenuItem { }
        background: Rectangle {
            color: sidebar.bright ? "#ffffff" : "#252526"
            border.color: sidebar.bright ? "#e5e5ea" : "#3a3a3b"
            radius: 8
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Статистика часу")
            visible: launcherController && launcherController.hasStatisticsAction()
            onTriggered: launcherController.on_actionStatistics_triggered()
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Donatello")
            visible: launcherController && launcherController.hasStatisticsAction()
            onTriggered: launcherController.on_actionDonatello_triggered()
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Дискорд")
            visible: launcherController && launcherController.hasDiscordAction()
            onTriggered: launcherController.on_actionDISCORD_triggered()
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Telegram")
            onTriggered: launcherController.on_actionTELEGRAM_triggered()
        }
        StyledMenuItem {
            text: launcherController ? launcherController.getAboutMenuLabel() : ""
            onTriggered: launcherController.on_actionAbout_triggered()
        }
    }

    Menu {
        id: accountMenu
        popupType: Popup.Window
        width: 280
        delegate: StyledMenuItem { }
        background: Rectangle {
            color: sidebar.bright ? "#ffffff" : "#252526"
            border.color: sidebar.bright ? "#e5e5ea" : "#3a3a3b"
            radius: 8
        }

        StyledMenuItem {
            text: qsTranslate("MainWindow", "No accounts added!")
            enabled: false
            visible: sidebar.accountMenuItems.length === 0
        }

        Instantiator {
            model: sidebar.accountMenuItems
            delegate: MenuItem {
                id: accountMenuItem
                required property var modelData
                text: modelData.text
                checkable: true
                checked: modelData.checked
                icon.source: modelData.icon
                implicitHeight: 40
                contentItem: RowLayout {
                    spacing: 8
                    Image {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        source: accountMenuItem.icon.source
                        fillMode: Image.PreserveAspectFit
                        visible: source.toString() !== ""
                    }
                    Text {
                        Layout.fillWidth: true
                        text: accountMenuItem.text
                        color: accountMenuItem.highlighted
                               ? sidebar.accentForeground
                               : (sidebar.bright ? "#1a1a1b" : "#ffffff")
                        font.pixelSize: 13
                        font.family: "Segoe UI"
                        elide: Text.ElideRight
                    }
                }
                background: Rectangle {
                    color: accountMenuItem.highlighted
                           ? sidebar.accentColor : "transparent"
                    radius: 4
                }
                onTriggered: launcherController.setDefaultAccount(modelData.index)
            }
            onObjectAdded: (index, object) => accountMenu.insertItem(index, object)
            onObjectRemoved: (index, object) => accountMenu.removeItem(object)
        }

        MenuSeparator { }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "No Default Account")
            checkable: true
            checked: launcherController ? !launcherController.hasDefaultAccount() : true
            icon.source: launcherController
                         ? launcherController.getThemedIconUrl("noaccount")
                         : ""
            onTriggered: launcherController.setDefaultAccount(-1)
        }
        MenuSeparator { }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Manage Accounts")
            onTriggered: launcherController.on_actionManageAccounts_triggered()
        }
    }

    component StyledMenuItem: MenuItem {
        id: styledMenuItem
        implicitHeight: 38
        contentItem: RowLayout {
            spacing: 8
            Image {
                Layout.preferredWidth: styledMenuItem.icon.source.toString() === "" ? 0 : 20
                Layout.preferredHeight: 20
                source: styledMenuItem.icon.source
                fillMode: Image.PreserveAspectFit
                visible: source.toString() !== ""
            }
            Text {
                Layout.fillWidth: true
                text: styledMenuItem.text
                color: !styledMenuItem.enabled
                       ? (sidebar.bright ? "#8e929a" : "#858991")
                       : (styledMenuItem.highlighted
                          ? sidebar.accentForeground
                          : (sidebar.bright ? "#202127" : "#f0f1f3"))
                font.pixelSize: 13
                font.family: "Segoe UI"
                elide: Text.ElideRight
            }
        }
        background: Rectangle {
            color: styledMenuItem.highlighted
                   ? sidebar.accentColor : "transparent"
            radius: 5
        }
    }

}
