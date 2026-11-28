import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Item {
    id: root

    required property var instanceData
    required property var controller
    required property bool bright
    required property string accentColor
    required property color accentTextColor
    required property int iconVersion
    required property bool selected

    signal selectRequested(int instanceIndex, string instanceId)
    signal contextMenuRequested(real x, real y)

    readonly property bool isRunning: instanceData && instanceData.isRunning ? true : false
    readonly property string gameVer: instanceData && instanceData.gameVersion ? instanceData.gameVersion : ""
    readonly property string modLoader: instanceData && instanceData.loader ? instanceData.loader : ""
    readonly property string playDuration: instanceData && instanceData.playTime ? instanceData.playTime : ""
    readonly property string coverUrl: instanceData && instanceData.screenshot ? instanceData.screenshot : ""

    scale: cardMouse.pressed ? 0.98 : (cardHover.hovered ? 1.025 : 1.0)
    Behavior on scale {
        NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
    }

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 4
        radius: Theme.radiusLg
        clip: true

        color: selected
               ? Theme.bgCardSelected
               : (cardHover.hovered ? Theme.bgCardHover : Theme.bgCard)

        border.color: isRunning
                      ? Theme.statusRunning
                      : (selected
                             ? Theme.accent
                             : (cardHover.hovered ? Theme.borderHover : Theme.borderSubtle))
        border.width: isRunning || selected ? 2 : 1

        Behavior on color { ColorAnimation { duration: Theme.animFast } }
        Behavior on border.color { ColorAnimation { duration: Theme.animFast } }

        // Running pulse animation on border
        SequentialAnimation on border.color {
            running: root.isRunning
            loops: Animation.Infinite
            ColorAnimation { from: Theme.statusRunning; to: Theme.alpha(Theme.statusRunning, 0.4); duration: 1000; easing.type: Easing.InOutQuad }
            ColorAnimation { from: Theme.alpha(Theme.statusRunning, 0.4); to: Theme.statusRunning; duration: 1000; easing.type: Easing.InOutQuad }
        }

        // --- HERO COVER BACKGROUND (if instance has screenshot) ---
        Item {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 90
            visible: root.coverUrl !== ""
            clip: true

            Image {
                anchors.fill: parent
                source: root.coverUrl
                fillMode: Image.PreserveAspectCrop
                opacity: cardHover.hovered ? 0.45 : 0.28
                Behavior on opacity { NumberAnimation { duration: Theme.animNormal } }
            }

            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "transparent" }
                    GradientStop { position: 0.7; color: Theme.alpha(card.color, 0.6) }
                    GradientStop { position: 1.0; color: card.color }
                }
            }
        }

        HoverHandler {
            id: cardHover
        }

        MouseArea {
            id: cardMouse
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            cursorShape: Qt.PointingHandCursor

            onClicked: (mouse) => {
                root.selectRequested(root.instanceData.modelIndex, root.instanceData.instanceId)
                if (mouse.button === Qt.RightButton && root.controller) {
                    root.contextMenuRequested(mouse.x, mouse.y)
                }
            }

            onDoubleClicked: {
                root.selectRequested(root.instanceData.modelIndex, root.instanceData.instanceId)
                if (root.controller) root.controller.on_actionLaunchInstance_triggered()
            }
        }

        // --- RUNNING BADGE ---
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 10
            height: 20
            radius: Theme.radiusPill
            color: Theme.alpha(Theme.statusRunning, 0.2)
            border.color: Theme.alpha(Theme.statusRunning, 0.6)
            border.width: 1
            visible: root.isRunning
            z: 2

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6
                anchors.rightMargin: 8
                spacing: 5

                Rectangle {
                    width: 7
                    height: 7
                    radius: 4
                    color: Theme.statusRunning

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { from: 1.0; to: 0.3; duration: 800; easing.type: Easing.InOutQuad }
                        NumberAnimation { from: 0.3; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                    }
                }

                Text {
                    text: qsTranslate("MainWindow", "Running")
                    color: Theme.statusRunning
                    font.pixelSize: 10
                    font.family: Theme.fontFamily
                    font.bold: true
                }
            }
        }

        // --- MAIN CONTENT ---
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 6

            // Icon Container
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                Layout.alignment: Qt.AlignHCenter

                Rectangle {
                    id: iconBg
                    anchors.centerIn: parent
                    width: 60
                    height: 60
                    radius: Theme.radiusXl
                    color: root.bright ? "#f4f4f5" : "#14171d"
                    border.color: root.selected ? Theme.accent : Theme.borderSubtle
                    border.width: 1

                    Behavior on border.color { ColorAnimation { duration: Theme.animFast } }

                    Image {
                        id: instanceIcon
                        anchors.centerIn: parent
                        source: root.instanceData.iconKey === "default" || root.instanceData.iconKey === "logo"
                                ? "image://themedicon/logo?v=" + root.iconVersion
                                : (root.instanceData.iconKey
                                   ? "image://themedicon/" + root.instanceData.iconKey + "?v=" + root.iconVersion
                                   : "")
                        width: 48
                        height: 48
                        fillMode: Image.PreserveAspectFit
                        sourceSize.width: 48
                        sourceSize.height: 48
                    }
                }
            }

            // Instance Name
            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: 34
                text: root.instanceData.display || ""
                color: Theme.textPrimary
                font.pixelSize: 14
                font.family: Theme.fontFamily
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            // Subtitle Pills (Version, Loader, Playtime)
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 6
                visible: root.gameVer !== "" || root.playDuration !== ""

                // Loader / Version Badge
                Rectangle {
                    Layout.preferredHeight: 18
                    implicitWidth: verText.implicitWidth + 12
                    radius: Theme.radiusPill
                    color: Theme.bgPill
                    border.color: Theme.borderSubtle
                    visible: root.gameVer !== ""

                    Text {
                        id: verText
                        anchors.centerIn: parent
                        text: (root.modLoader ? root.modLoader + " " : "") + root.gameVer
                        color: Theme.textSecondary
                        font.pixelSize: 10
                        font.family: Theme.fontFamily
                        font.bold: true
                    }
                }

                // Playtime Badge
                Rectangle {
                    Layout.preferredHeight: 18
                    implicitWidth: playText.implicitWidth + 12
                    radius: Theme.radiusPill
                    color: Theme.bgPill
                    border.color: Theme.borderSubtle
                    visible: root.playDuration !== ""

                    Text {
                        id: playText
                        anchors.centerIn: parent
                        text: root.playDuration
                        color: Theme.textMuted
                        font.pixelSize: 10
                        font.family: Theme.fontFamily
                    }
                }
            }

            Item { Layout.fillHeight: true }

            // Action Buttons Overlay
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                spacing: 6
                opacity: cardHover.hovered || root.selected ? 1 : 0
                enabled: cardHover.hovered || root.selected

                Behavior on opacity { NumberAnimation { duration: Theme.animFast } }

                // Launch Button
                Button {
                    id: playButton
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    background: Rectangle {
                        color: playButton.hovered
                               ? Theme.accent
                               : (root.bright ? "#e5e7eb" : "#2a2e39")
                        radius: Theme.radiusMd
                        border.color: playButton.hovered ? Theme.accent : Theme.borderSubtle
                        Behavior on color { ColorAnimation { duration: 100 } }
                    }
                    contentItem: RowLayout {
                        spacing: 4
                        Item { Layout.fillWidth: true }
                        Image {
                            id: playImage
                            Layout.preferredWidth: 14
                            Layout.preferredHeight: 14
                            source: root.controller ? root.controller.getThemedIconUrl("status-running") + "?v=" + root.iconVersion : ""
                            visible: false
                        }
                        ColorOverlay {
                            Layout.preferredWidth: 14
                            Layout.preferredHeight: 14
                            source: playImage
                            color: playButton.hovered ? Theme.textOnAccent : Theme.textPrimary
                        }
                        Text {
                            text: qsTranslate("MainWindow", "Play")
                            color: playButton.hovered ? Theme.textOnAccent : Theme.textPrimary
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.bold: true
                        }
                        Item { Layout.fillWidth: true }
                    }
                    onClicked: {
                        root.selectRequested(root.instanceData.modelIndex, root.instanceData.instanceId)
                        if (root.controller) root.controller.on_actionLaunchInstance_triggered()
                    }
                }

                // Edit Button
                Button {
                    id: settingsButton
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    background: Rectangle {
                        color: settingsButton.hovered ? (root.bright ? "#d1d5db" : "#323744") : (root.bright ? "#e5e7eb" : "#2a2e39")
                        border.color: Theme.borderSubtle
                        radius: Theme.radiusMd
                    }
                    contentItem: Item {
                        Image {
                            id: settingsImage
                            anchors.centerIn: parent
                            source: root.controller ? root.controller.getThemedIconUrl("settings") + "?v=" + root.iconVersion : ""
                            width: 14
                            height: 14
                            visible: false
                        }
                        ColorOverlay {
                            anchors.fill: settingsImage
                            source: settingsImage
                            color: Theme.textSecondary
                        }
                    }
                    onClicked: {
                        root.selectRequested(root.instanceData.modelIndex, root.instanceData.instanceId)
                        if (root.controller) root.controller.on_actionEditInstance_triggered()
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: qsTranslate("MainWindow", "Edit Instance")
                }

                // Folder Button
                Button {
                    id: folderButton
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    background: Rectangle {
                        color: folderButton.hovered ? (root.bright ? "#d1d5db" : "#323744") : (root.bright ? "#e5e7eb" : "#2a2e39")
                        border.color: Theme.borderSubtle
                        radius: Theme.radiusMd
                    }
                    contentItem: Item {
                        Image {
                            id: folderImage
                            anchors.centerIn: parent
                            source: root.controller ? root.controller.getThemedIconUrl("viewfolder") + "?v=" + root.iconVersion : ""
                            width: 14
                            height: 14
                            visible: false
                        }
                        ColorOverlay {
                            anchors.fill: folderImage
                            source: folderImage
                            color: Theme.textSecondary
                        }
                    }
                    onClicked: {
                        root.selectRequested(root.instanceData.modelIndex, root.instanceData.instanceId)
                        if (root.controller) root.controller.on_actionViewSelectedMCFolder_triggered()
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: qsTranslate("MainWindow", "Folder")
                }
            }
        }
    }
}
