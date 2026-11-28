pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar

    implicitWidth: 54
    color: Theme.bgSidebar

    property bool bright: launcherController
                          ? launcherController.themeName === "bright"
                            || launcherController.themeName === "system"
                          : false
    property string accentColor: launcherController
                                 ? launcherController.accentColor
                                 : "#ffae00"
    property color accentForeground: Theme.contrastForeground(accentColor)
    property int iconVersion: 0
    property var accountMenuItems: []

    function openMenu(menu, button) {
        const position = button.mapToItem(sidebar, 0, button.height)
        menu.x = width + 4
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

    // Right subtle separator border
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: Theme.borderSubtle
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        spacing: 12

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

        // User Account Button
        ToolbarButton {
            id: accountsButton
            Layout.alignment: Qt.AlignHCenter
            bright: sidebar.bright
            accentColor: sidebar.accentColor
            accentForeground: sidebar.accentForeground
            iconVersion: sidebar.iconVersion
            iconSize: 26
            iconSource: launcherController
                        ? (launcherController.accountIconUrl !== ""
                           ? launcherController.accountIconUrl
                           : launcherController.getThemedIconUrl("accounts"))
                        : ""
            tooltip: qsTranslate("MainWindow", "Manage Accounts")
            onClicked: sidebar.openAccountsMenu(accountsButton)
        }
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
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            radius: Theme.radiusMd
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
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            radius: Theme.radiusMd
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
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            radius: Theme.radiusMd
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
                               ? Theme.textOnAccent
                               : Theme.textPrimary
                        font.pixelSize: Theme.fontSizeBody
                        font.family: Theme.fontFamily
                        elide: Text.ElideRight
                    }
                }
                background: Rectangle {
                    color: accountMenuItem.highlighted
                           ? Theme.accent : "transparent"
                    radius: Theme.radiusSm
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
                       ? Theme.textMuted
                       : (styledMenuItem.highlighted
                          ? Theme.textOnAccent
                          : Theme.textPrimary)
                font.pixelSize: Theme.fontSizeBody
                font.family: Theme.fontFamily
                elide: Text.ElideRight
            }
        }
        background: Rectangle {
            color: styledMenuItem.highlighted
                   ? Theme.accent : "transparent"
            radius: Theme.radiusSm
        }
    }
}
