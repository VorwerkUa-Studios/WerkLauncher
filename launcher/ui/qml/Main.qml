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

            // --- LEFT SIDEBAR (Main Toolbar, matches original LeftToolBarArea) ---
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 54
                color: isBright ? "#ffffff" : "transparent" // Solid white on bright

                // Right border for the sidebar
                Rectangle {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 1
                    color: isBright ? "#e5e5ea" : Qt.rgba(1, 1, 1, 0.06) 
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.topMargin: 12
                    anchors.bottomMargin: 6
                    spacing: 16

                    // Spacer to align with search bar (34px in original)
                    Item { Layout.preferredHeight: 34 }

                    // Add Instance
                    ToolbarButton {
                        iconSource: launcherController ? launcherController.getThemedIconUrl("new") : ""
                        tooltip: qsTranslate("MainWindow", "Add Instance")
                        onClicked: launcherController.on_actionAddInstance_triggered()
                    }

                    // Folders
                    ToolbarButton {
                        iconSource: launcherController ? launcherController.getThemedIconUrl("viewfolder") : ""
                        tooltip: qsTranslate("MainWindow", "Folders")
                        onClicked: launcherController.on_actionViewInstanceFolder_triggered()
                    }

                    // Settings
                    ToolbarButton {
                        iconSource: launcherController ? launcherController.getThemedIconUrl("settings") : ""
                        tooltip: qsTranslate("MainWindow", "Settings")
                        onClicked: launcherController.on_actionSettings_triggered()
                    }

                    // Help (with submenu)
                    ToolbarButton {
                        id: helpButton
                        iconSource: launcherController ? launcherController.getThemedIconUrl("help") : ""
                        tooltip: qsTranslate("MainWindow", "Help")
                        onClicked: launcherController.showHelpMenu()
                    }

                    // Spacer
                    Item { Layout.fillHeight: true }

                    // Accounts (bottom)
                    ToolbarButton {
                        iconSource: launcherController ? (launcherController.accountIconUrl !== "" ? launcherController.accountIconUrl : launcherController.getThemedIconUrl("accounts")) : ""
                        tooltip: qsTranslate("MainWindow", "Manage Accounts")
                        iconSize: 28
                        onClicked: launcherController.on_actionManageAccounts_triggered()
                    }

                    Item { Layout.preferredHeight: 8 }
                }
            } // End of Left Sidebar

            // --- MAIN VIEW ---
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // Top Toolbar (Search + Sort)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
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
                            Layout.preferredWidth: 140
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
                                width: sortCombo.width
                                contentItem: Text {
                                    text: modelData
                                    color: isBright ? (highlighted ? "#ffffff" : "#1a1a1b") : (highlighted ? "#000000" : "#ffffff")
                                    font.pixelSize: 12
                                    font.family: "Segoe UI"
                                }
                                background: Rectangle {
                                    color: highlighted ? mainWindow.accentColor : "transparent"
                                }
                            }

                            popup: Popup {
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

                    // Background MouseArea for empty space right-clicks
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.RightButton
                        onClicked: (mouse) => {
                            if (mouse.button === Qt.RightButton) {
                                if (launcherController) launcherController.showEmptySpaceContextMenu()
                            }
                        }
                    }

                    Flickable {
                        id: instanceGroupsView
                        anchors.fill: parent
                        anchors.margins: 16
                        clip: true
                        contentWidth: width
                        contentHeight: groupsColumn.implicitHeight

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        Column {
                            id: groupsColumn
                            width: instanceGroupsView.width
                            spacing: 16

                            Repeater {
                                id: groupRepeater
                                model: mainWindow.instanceGroups

                                delegate: Column {
                                    id: groupSection
                                    required property var modelData
                                    property bool collapsed: modelData.collapsed
                                    width: groupsColumn.width
                                    spacing: 4

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
                                            font.pixelSize: 14
                                            font.family: "Segoe UI"
                                            font.bold: true
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                                            onClicked: (mouse) => {
                                                if (mouse.button === Qt.RightButton) {
                                                    if (launcherController) {
                                                        launcherController.showGroupContextMenu(modelData.name)
                                                    }
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
                                        spacing: 0
                                        implicitHeight: childrenRect.height
                                        visible: !groupSection.collapsed

                                        Repeater {
                                            model: modelData.instances

                                            delegate: InstanceCard {
                                                required property var modelData
                                                instanceData: modelData
                                                controller: launcherController
                                                bright: mainWindow.isBright
                                                accentColor: mainWindow.accentColor
                                                iconVersion: mainWindow.iconVersion
                                                selected: instanceData.modelIndex === mainWindow.selectedInstanceIndex
                                                onSelectRequested: (instanceIndex, instanceId) => mainWindow.selectInstance(instanceIndex, instanceId)
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

    // Component for styled menus
    component StyledMenu : Menu {
        background: Rectangle {
            color: isBright ? "#ffffff" : "#252526"
            border.color: isBright ? "#e5e5ea" : "#2a2a2b"
            radius: 8
        }
        
        delegate: MenuItem {
            id: menuItem
            contentItem: Text {
                text: menuItem.text
                color: isBright ? (menuItem.highlighted ? "#ffffff" : "#1a1a1b") : (menuItem.highlighted ? "#000000" : "#ffffff")
                font.pixelSize: 13
                font.family: "Segoe UI"
            }
            background: Rectangle {
                color: menuItem.highlighted ? mainWindow.accentColor : "transparent"
                radius: 4
            }
        }
    }

    // Help Menu Top-Level (Native C++ menu used instead)

    // Reusable Sidebar Toolbar Button
    component ToolbarButton: Item {
        id: control
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40
        Layout.alignment: Qt.AlignHCenter
        property string iconSource: ""
        property string tooltip: ""
        property int iconSize: 24

        signal clicked()

        Rectangle {
            anchors.fill: parent
            radius: 6
            color: btnMouseArea.containsMouse ? (isBright ? "#f2f2f7" : "#333333") : (isBright ? "#ffffff" : "#252526")
            border.color: btnMouseArea.containsMouse ? mainWindow.accentColor : (isBright ? "#d1d1d6" : "#333333")
            border.width: 1

            Behavior on color {
                ColorAnimation { duration: 100 }
            }
            
            Behavior on border.color {
                ColorAnimation { duration: 100 }
            }

            Image {
                anchors.centerIn: parent
                source: control.iconSource + "?v=" + iconVersion
                width: control.iconSize
                height: control.iconSize
                fillMode: Image.PreserveAspectFit
                opacity: 1.0
            }
        }

        ToolTip {
            id: btnTooltip
            text: control.tooltip
            visible: btnMouseArea.containsMouse && control.tooltip !== ""
            delay: 500
            
            contentItem: Text {
                text: btnTooltip.text
                color: "#000000" // Tooltips in this app are always black text on accent bg
                font.pixelSize: 12
                font.family: "Segoe UI"
            }
            
            background: Rectangle {
                color: mainWindow.accentColor
                radius: 4
            }
        }

        MouseArea {
            id: btnMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }

    // The C++ MainWindow handles borders, shadows, and resizing natively.
}
