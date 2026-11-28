pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: mainWindow
    anchors.fill: parent

    property string themeName: launcherController ? launcherController.themeName : "dark"
    property bool isBright: themeName === "bright" || themeName === "system"

    Binding {
        target: Theme
        property: "isBright"
        value: mainWindow.isBright
    }
    Binding {
        target: Theme
        property: "accent"
        value: Qt.color(mainWindow.accentColor)
    }

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

    color: Theme.bgRoot

    property string accentColor: launcherController ? launcherController.accentColor : "#ffae00"
    property color accentForeground: Theme.contrastForeground(accentColor)

    // Custom background wallpaper support
    Image {
        anchors.fill: parent
        source: launcherController ? (launcherController.getWallpaper() || "") : ""
        fillMode: Image.PreserveAspectCrop
        visible: source.toString() !== ""
        opacity: mainWindow.isBright ? 0.08 : 0.15
    }

    property int selectedInstanceIndex: -1
    property var instanceGroups: []
    property bool initializingSortMode: true
    property int instanceCardWidth: Math.max(160, Math.min(220, (width - 80) / 4))
    property int instanceCardHeight: Math.round(instanceCardWidth * 1.25)

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
                    Layout.preferredHeight: 52
                    color: "transparent"

                    // Subtle bottom separator
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 1
                        color: Theme.borderSubtle
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        anchors.topMargin: 8
                        anchors.bottomMargin: 8
                        spacing: 12

                        // Search Field
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            color: Theme.bgInput
                            radius: Theme.radiusMd
                            border.color: searchInput.activeFocus ? Theme.accent : Theme.borderSubtle
                            border.width: 1

                            Behavior on border.color { ColorAnimation { duration: Theme.animFast } }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 8

                                Text {
                                    text: "\uD83D\uDD0D" // magnifying glass
                                    font.pixelSize: 12
                                    opacity: 0.6
                                }

                                TextInput {
                                    id: searchInput
                                    Layout.fillWidth: true
                                    verticalAlignment: TextInput.AlignVCenter
                                    color: Theme.textPrimary
                                    selectionColor: Theme.accent
                                    selectedTextColor: Theme.textOnAccent
                                    font.pixelSize: Theme.fontSizeBody
                                    font.family: Theme.fontFamily
                                    clip: true

                                    onTextChanged: {
                                        if (launcherController) {
                                            launcherController.searchTextChanged(text)
                                        }
                                    }

                                    Text {
                                        text: qsTranslate("MainWindow", "Search instances...")
                                        color: Theme.textMuted
                                        font.pixelSize: Theme.fontSizeBody
                                        font.family: Theme.fontFamily
                                        anchors.verticalCenter: parent.verticalCenter
                                        visible: !parent.text && !parent.activeFocus
                                    }
                                }

                                // Clear Button when text entered
                                Text {
                                    text: "\u2715"
                                    color: Theme.textMuted
                                    font.pixelSize: 12
                                    font.bold: true
                                    visible: searchInput.text.length > 0
                                    MouseArea {
                                        anchors.fill: parent
                                        anchors.margins: -4
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            searchInput.text = ""
                                            searchInput.forceActiveFocus()
                                        }
                                    }
                                }

                                // Shortcut Hint Badge
                                Rectangle {
                                    visible: searchInput.text.length === 0
                                    Layout.preferredHeight: 18
                                    implicitWidth: 42
                                    radius: Theme.radiusXs
                                    color: Theme.bgPill
                                    border.color: Theme.borderSubtle
                                    border.width: 1

                                    Text {
                                        anchors.centerIn: parent
                                        text: "Ctrl+F"
                                        color: Theme.textMuted
                                        font.pixelSize: 10
                                        font.family: Theme.fontMono
                                    }
                                }
                            }
                        }

                        // Sort ComboBox
                        ComboBox {
                            id: sortCombo
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 36
                            model: [qsTranslate("MainWindow", "Sort by Name"), qsTranslate("MainWindow", "Sort by Last Played"), qsTranslate("MainWindow", "Sort by Version")]
                            currentIndex: 0

                            background: Rectangle {
                                color: Theme.bgInput
                                radius: Theme.radiusMd
                                border.color: sortCombo.activeFocus || sortCombo.pressed ? Theme.accent : Theme.borderSubtle
                                border.width: 1
                            }

                            contentItem: Text {
                                text: sortCombo.displayText
                                color: Theme.textPrimary
                                font.pixelSize: Theme.fontSizeSmall
                                font.family: Theme.fontFamily
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 10
                            }

                            delegate: ItemDelegate {
                                id: sortItem
                                required property string modelData
                                width: sortCombo.width
                                implicitHeight: 34
                                contentItem: Text {
                                    text: sortItem.modelData
                                    color: sortItem.highlighted
                                           ? Theme.textOnAccent
                                           : Theme.textPrimary
                                    font.pixelSize: Theme.fontSizeSmall
                                    font.family: Theme.fontFamily
                                }
                                background: Rectangle {
                                    color: sortItem.highlighted ? Theme.accent : "transparent"
                                    radius: Theme.radiusSm
                                }
                            }

                            popup: Popup {
                                popupType: Popup.Item
                                y: sortCombo.height + 4
                                width: sortCombo.width
                                implicitHeight: contentItem.implicitHeight + 8
                                padding: 4

                                contentItem: ListView {
                                    clip: true
                                    implicitHeight: contentHeight
                                    model: sortCombo.popup.visible ? sortCombo.delegateModel : null
                                    currentIndex: sortCombo.highlightedIndex
                                    ScrollIndicator.vertical: ScrollIndicator { }
                                }

                                background: Rectangle {
                                    color: Theme.bgCard
                                    border.color: Theme.borderSubtle
                                    radius: Theme.radiusMd
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
                        anchors.margins: 16
                        clip: true
                        contentWidth: width
                        contentHeight: groupsColumn.implicitHeight

                        ScrollBar.vertical: ScrollBar {
                            id: vScrollBar
                            policy: ScrollBar.AsNeeded
                            width: 6
                            background: null
                            contentItem: Rectangle {
                                implicitWidth: 6
                                radius: 3
                                color: vScrollBar.hovered || vScrollBar.pressed
                                       ? Theme.accent
                                       : (Theme.isBright ? "rgba(0, 0, 0, 0.25)" : "rgba(255, 255, 255, 0.25)")
                                opacity: vScrollBar.active ? 1.0 : 0.0
                                Behavior on opacity { NumberAnimation { duration: 250 } }
                            }
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
                            spacing: 24

                            Repeater {
                                id: groupRepeater
                                model: mainWindow.instanceGroups

                                delegate: Column {
                                    id: groupSection
                                    required property var modelData
                                    property bool collapsed: modelData.collapsed
                                    width: groupsColumn.width
                                    spacing: 12

                                    // Group Header
                                    Row {
                                        spacing: 8

                                        Text {
                                            text: "\u25B6"
                                            color: Theme.accent
                                            font.pixelSize: 10
                                            rotation: groupSection.collapsed ? 0 : 90
                                            anchors.verticalCenter: parent.verticalCenter
                                            Behavior on rotation {
                                                NumberAnimation { duration: Theme.animFast; easing.type: Easing.OutQuad }
                                            }
                                        }

                                        Text {
                                            text: modelData.label
                                            color: Theme.textPrimary
                                            font.pixelSize: Theme.fontSizeSubheading
                                            font.family: Theme.fontFamily
                                            font.bold: true
                                        }

                                        // Instances Count Badge
                                        Rectangle {
                                            anchors.verticalCenter: parent.verticalCenter
                                            height: 18
                                            implicitWidth: countText.implicitWidth + 10
                                            radius: Theme.radiusPill
                                            color: Theme.bgPill
                                            border.color: Theme.borderSubtle
                                            border.width: 1

                                            Text {
                                                id: countText
                                                anchors.centerIn: parent
                                                text: modelData.instances.length
                                                color: Theme.textMuted
                                                font.pixelSize: 10
                                                font.family: Theme.fontFamily
                                                font.bold: true
                                            }
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                                            cursorShape: Qt.PointingHandCursor
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
                                        spacing: 14
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
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSizeHeading
                            font.family: Theme.fontFamily
                            visible: (!instanceModel || instanceModel.rowCount() === 0) && mainWindow.instanceGroups.length === 0
                        }
                    }
                }
            }
        }

        // --- STATUS BAR ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 26
            color: "transparent"

            // Top border
            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: Theme.borderSubtle
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                Text {
                    text: instanceModel ? instanceModel.rowCount() + " " + qsTranslate("MainWindow", "instance(s)") : ""
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeCaption
                    font.family: Theme.fontFamily
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: ""
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeCaption
                    font.family: Theme.fontFamily
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
                color: Theme.bgCard
                border.color: Theme.borderSubtle
                radius: Theme.radiusMd
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
                   ? Theme.accent
                   : "transparent"
            radius: Theme.radiusSm
        }
    }

    component StyledMenu: Menu {
        parent: mainWindow
        popupType: Popup.Item
        delegate: StyledMenuItem { }
        background: Rectangle {
            color: Theme.bgCard
            border.color: Theme.borderSubtle
            radius: Theme.radiusMd
        }
    }
}
