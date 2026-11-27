pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: mainWindow
    anchors.fill: parent
    
    property string themeName: launcherController ? launcherController.themeName : "dark"
    property bool isBright: themeName === "bright" || themeName === "system" // Assuming bright if not dark, or we could just check if it's "bright"
    
    // Dynamic icon reload support
    property int iconVersion: 0
    Connections {
        target: launcherController
        function onIsShowingSettingsChanged(isShowing) {
            if (!isShowing) {
                iconVersion++
            }
        }
    }
    
    color: isBright ? "#f5f5f7" : "#1a1a1b"

    property string accentColor: launcherController ? launcherController.accentColor : "#4CAF50"
    property color accentForeground: contrastForeground(accentColor)
    
    // Custom background wallpaper support
    Image {
        anchors.fill: parent
        source: launcherController ? (launcherController.getWallpaper() || "") : "" // Fallback if C++ doesn't supply it yet
        fillMode: Image.PreserveAspectCrop
        visible: source.toString() !== ""
        opacity: 0.2 // subtle background
    }
    property int selectedInstanceIndex: -1
    property var instanceGroups: []
    property bool initializingSortMode: true
    property int instanceCardWidth: Math.max(128, Math.min(176, (width - 64) / 5))
    property int instanceCardHeight: instanceCardWidth

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
        const whiteContrast = 1.05 / (luminance + 0.05)
        const blackContrast = (luminance + 0.05) / 0.05
        return whiteContrast >= blackContrast ? "#ffffff" : "#000000"
    }

    function openMenuAt(menu, x, y) {
        const menuWidth = menu.implicitWidth > 0 ? menu.implicitWidth : menu.width
        const menuHeight = menu.implicitHeight > 0 ? menu.implicitHeight : menu.height
        menu.x = Math.max(0, Math.min(x, width - menuWidth))
        menu.y = Math.max(0, Math.min(y, height - menuHeight))
        menu.open()
    }

    function selectInstance(index, instId) {
        mainWindow.selectedInstanceIndex = index
        if (launcherController) {
            launcherController.setSelectedInstanceById(instId)
        }
    }

    function refreshInstanceGroups() {
        instanceGroups = launcherController ? launcherController.getInstanceGroups() : []
    }

    Component.onCompleted: {
        refreshInstanceGroups()
        if (launcherController) {
            sortCombo.currentIndex = launcherController.getSortModeIndex()
        }
        initializingSortMode = false
    }

    Connections {
        target: instanceModel
        function onDataChanged() { mainWindow.refreshInstanceGroups() }
        function onRowsInserted() { mainWindow.refreshInstanceGroups() }
        function onRowsRemoved() { mainWindow.refreshInstanceGroups() }
        function onLayoutChanged() { mainWindow.refreshInstanceGroups() }
        function onModelReset() { mainWindow.refreshInstanceGroups() }
    }

    // Main Layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- CONTENT AREA ---
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // --- MAIN VIEW ---
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // Top Toolbar (Search + Sort)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: mainWindow.width < 820 ? 44 : 50
                    color: "transparent"

                    // Subtle bottom border
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 1
                        color: isBright ? "#d1d1d6" : Qt.rgba(1, 1, 1, 0.06)
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        anchors.topMargin: 8
                        anchors.bottomMargin: 8
                        spacing: 8

                        // Search Field
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 32
                            color: isBright ? "rgba(255, 255, 255, 0.8)" : Qt.rgba(0, 0, 0, 0.2)
                            radius: 4
                            border.color: searchInput.activeFocus ? mainWindow.accentColor : (isBright ? "#d1d1d6" : Qt.rgba(1, 1, 1, 0.1))

                            TextInput {
                                id: searchInput
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                verticalAlignment: TextInput.AlignVCenter
                                color: isBright ? "#1a1a1b" : "#dddddd"
                                font.pixelSize: 13
                                font.family: "Segoe UI"
                                clip: true

                                onTextChanged: {
                                    if (launcherController) {
                                        launcherController.searchTextChanged(text)
                                    }
                                }

                                Text {
                                    text: qsTranslate("MainWindow", "Search instances...")
                                    color: isBright ? "#8e8e93" : "#555555"
                                    font.pixelSize: 13
                                    font.family: "Segoe UI"
                                    anchors.verticalCenter: parent.verticalCenter
                                    visible: !parent.text && !parent.activeFocus
                                }
                            }
                        }

                        // Sort ComboBox
                        ComboBox {
                            id: sortCombo
                            Layout.preferredWidth: mainWindow.width < 820 ? 112 : 148
                            Layout.preferredHeight: 32
                            model: [qsTranslate("MainWindow", "Sort by Name"), qsTranslate("MainWindow", "Sort by Last Played"), qsTranslate("MainWindow", "Sort by Version")]
                            currentIndex: 0

                            background: Rectangle {
                                color: isBright ? "#ffffff" : Qt.rgba(0, 0, 0, 0.2)
                                radius: 4
                                border.color: sortCombo.pressed ? mainWindow.accentColor : (isBright ? "#d1d1d6" : Qt.rgba(1, 1, 1, 0.1))
                            }

                            contentItem: Text {
                                text: sortCombo.displayText
                                color: isBright ? "#1a1a1b" : "#aaaaaa"
                                font.pixelSize: 12
                                font.family: "Segoe UI"
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 12
                            }
                            
                            delegate: ItemDelegate {
                                id: sortItem
                                required property string modelData
                                width: sortCombo.width
                                implicitHeight: 36
                                contentItem: Text {
                                    text: sortItem.modelData
                                    color: sortItem.highlighted
                                           ? mainWindow.accentForeground
                                           : (mainWindow.isBright ? "#202127" : "#f0f1f3")
                                    font.pixelSize: 12
                                    font.family: "Segoe UI"
                                }
                                background: Rectangle {
                                    color: sortItem.highlighted ? mainWindow.accentColor : "transparent"
                                }
                            }

                            popup: Popup {
                                popupType: Popup.Item
                                y: sortCombo.height - 1
                                width: sortCombo.width
                                implicitHeight: contentItem.implicitHeight
                                padding: 1

                                contentItem: ListView {
                                    clip: true
                                    implicitHeight: contentHeight
                                    model: sortCombo.popup.visible ? sortCombo.delegateModel : null
                                    currentIndex: sortCombo.highlightedIndex
                                    ScrollIndicator.vertical: ScrollIndicator { }
                                }

                                background: Rectangle {
                                    color: isBright ? "#ffffff" : "#252526"
                                    border.color: isBright ? "#e5e5ea" : "#2a2a2b"
                                    radius: 4
                                }
                            }

                            onCurrentIndexChanged: {
                                if (!mainWindow.initializingSortMode && launcherController) {
                                    launcherController.sortModeChanged(currentIndex)
                                }
                            }
                        }
                    }
                }

                // Instance Grid Area
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"

                    Flickable {
                        id: instanceGroupsView
                        anchors.fill: parent
                        anchors.margins: Math.max(8, Math.min(18, mainWindow.width / 80))
                        clip: true
                        contentWidth: width
                        contentHeight: groupsColumn.implicitHeight

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onTapped: searchInput.focus = false
                        }

                        MouseArea {
                            width: instanceGroupsView.width
                            height: Math.max(instanceGroupsView.height,
                                             instanceGroupsView.contentHeight)
                            acceptedButtons: Qt.RightButton
                            preventStealing: false
                            onClicked: (mouse) => {
                                const position = mapToItem(mainWindow, mouse.x, mouse.y)
                                mainWindow.openMenuAt(emptySpaceMenu, position.x, position.y)
                            }
                        }

                        Column {
                            id: groupsColumn
                            width: instanceGroupsView.width
                            spacing: 18

                            Repeater {
                                id: groupRepeater
                                model: mainWindow.instanceGroups

                                delegate: Column {
                                    id: groupSection
                                    required property var modelData
                                    property bool collapsed: modelData.collapsed
                                    width: groupsColumn.width
                                    spacing: 8

                                    Row {
                                        spacing: 6

                                        Text {
                                            text: groupSection.collapsed ? "\u25B6" : "\u25BC"
                                            color: isBright ? "#8e8e93" : "#999999"
                                            font.pixelSize: 11
                                            anchors.verticalCenter: parent.verticalCenter
                                        }

                                        Text {
                                            text: modelData.label
                                            color: isBright ? "#8e8e93" : "#999999"
                                            font.pixelSize: 15
                                            font.family: "Segoe UI"
                                            font.bold: true
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                                            onClicked: (mouse) => {
                                                if (mouse.button === Qt.RightButton) {
                                                    const position = mapToItem(mainWindow, mouse.x, mouse.y)
                                                    groupContextMenu.groupName = modelData.name
                                                    mainWindow.openMenuAt(groupContextMenu, position.x, position.y)
                                                } else if (modelData.name !== "") {
                                                    groupSection.collapsed = !groupSection.collapsed
                                                    if (launcherController) {
                                                        launcherController.setGroupCollapsed(modelData.name, groupSection.collapsed)
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Flow {
                                        width: parent.width
                                        spacing: 8
                                        height: childrenRect.height
                                        visible: !groupSection.collapsed

                                        Repeater {
                                            model: modelData.instances

                                            delegate: InstanceCard {
                                                id: instanceCard
                                                required property var modelData
                                                width: mainWindow.instanceCardWidth
                                                height: mainWindow.instanceCardHeight
                                                instanceData: modelData
                                                controller: launcherController
                                                bright: mainWindow.isBright
                                                accentColor: mainWindow.accentColor
                                                accentTextColor: mainWindow.accentForeground
                                                iconVersion: mainWindow.iconVersion
                                                selected: instanceData.modelIndex === mainWindow.selectedInstanceIndex
                                                onSelectRequested: (instanceIndex, instanceId) => mainWindow.selectInstance(instanceIndex, instanceId)
                                                onContextMenuRequested: (x, y) => {
                                                    const position = instanceCard.mapToItem(mainWindow, x, y)
                                                    instanceContextMenu.instanceName = instanceData.display
                                                    mainWindow.openMenuAt(instanceContextMenu, position.x, position.y)
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: instanceGroupsView
                            text: qsTranslate("MainWindow", "No instances found")
                            color: isBright ? "#8e8e93" : "#444444"
                            font.pixelSize: 16
                            font.family: "Segoe UI"
                            visible: (!instanceModel || instanceModel.rowCount() === 0) && mainWindow.instanceGroups.length === 0
                        }
                    }
                }
            }
        }

        // --- STATUS BAR ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 22
            color: "transparent"

            // Top border
            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: isBright ? "#e5e5ea" : Qt.rgba(1, 1, 1, 0.06)
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Text {
                    text: instanceModel ? instanceModel.rowCount() + " instance(s)" : ""
                    color: isBright ? "#8e8e93" : Qt.rgba(1, 1, 1, 0.45)
                    font.pixelSize: 11
                    font.family: "Segoe UI"
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: ""
                    color: Qt.rgba(1, 1, 1, 0.45)
                    font.pixelSize: 11
                    font.family: "Segoe UI"
                }
            }
        }
    }

    StyledMenu {
        id: emptySpaceMenu
        width: 230

        StyledMenuItem {
            text: qsTranslate("MainWindow", "Create instance")
            onTriggered: launcherController.on_actionAddInstance_triggered()
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Create group")
            onTriggered: launcherController.createGroupFromQml()
        }
    }

    StyledMenu {
        id: groupContextMenu
        property string groupName: ""
        width: 250

        StyledMenuItem {
            text: qsTranslate("MainWindow", "Create instance")
            onTriggered: launcherController.addInstanceToGroup(groupContextMenu.groupName)
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Delete group '%1'").arg(groupContextMenu.groupName)
            onTriggered: launcherController.deleteGroupFromQml(groupContextMenu.groupName)
        }
    }

    StyledMenu {
        id: instanceContextMenu
        property string instanceName: ""
        width: 250

        StyledMenuItem {
            text: instanceContextMenu.instanceName
            enabled: false
        }
        MenuSeparator { }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Change Icon")
            onTriggered: launcherController.on_actionChangeInstIcon_triggered()
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Change Group")
            onTriggered: launcherController.on_actionChangeInstGroup_triggered()
        }
        MenuSeparator { }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Edit Instance")
            onTriggered: launcherController.on_actionEditInstance_triggered()
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Minecraft Folder")
            onTriggered: launcherController.on_actionViewSelectedMCFolder_triggered()
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Config Folder")
            onTriggered: launcherController.on_actionConfig_Folder_triggered()
        }
        MenuSeparator { }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Create Shortcut")
            onTriggered: launcherController.on_actionCreateShortcut_triggered()
        }
        Menu {
            title: qsTranslate("MainWindow", "Export")
            popupType: Popup.Item
            delegate: StyledMenuItem { }
            background: Rectangle {
                color: mainWindow.isBright ? "#ffffff" : "#252526"
                border.color: mainWindow.isBright ? "#e5e7eb" : "#3a3a3b"
                radius: 8
            }
            StyledMenuItem {
                text: qsTranslate("MainWindow", "Export instance...")
                onTriggered: launcherController.on_actionExportInstance_triggered()
            }
            StyledMenuItem {
                text: qsTranslate("MainWindow", "Modrinth (WIP)")
                onTriggered: launcherController.exportSelectedInstanceToModrinth()
            }
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Delete")
            onTriggered: launcherController.on_actionDeleteInstance_triggered()
        }
        StyledMenuItem {
            text: qsTranslate("MainWindow", "Copy Instance")
            onTriggered: launcherController.on_actionCopyInstance_triggered()
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
                       ? (mainWindow.isBright ? "#8e929a" : "#858991")
                       : (styledMenuItem.highlighted
                              ? mainWindow.accentForeground
                              : (mainWindow.isBright ? "#202127" : "#f0f1f3"))
                font.pixelSize: 13
                font.family: "Segoe UI"
                elide: Text.ElideRight
            }
        }
        background: Rectangle {
            color: styledMenuItem.highlighted
                   ? mainWindow.accentColor
                   : "transparent"
            radius: 5
        }
    }

    component StyledMenu: Menu {
        parent: mainWindow
        popupType: Popup.Item
        delegate: StyledMenuItem { }
        background: Rectangle {
            color: mainWindow.isBright ? "#ffffff" : "#252526"
            border.color: mainWindow.isBright ? "#e5e5ea" : "#3a3a3b"
            radius: 8
        }
    }

    // The C++ MainWindow handles borders, shadows, and resizing natively.
}
