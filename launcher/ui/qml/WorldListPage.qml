pragma ComponentBehavior: Bound

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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                Text {
                    text: root.pageController.pageTitle
                    textFormat: Text.PlainText
                    color: palette.windowText
                    font.pixelSize: 22
                    font.bold: true
                }

                Text {
                    text: qsTranslate("WorldListPage", "%1 worlds")
                          .arg(root.pageController.worlds.length)
                    color: palette.windowText
                    opacity: 0.7
                    font.pixelSize: 13
                }
            }

            Label {
                text: qsTranslate("WorldListPage", "Sort by")
            }

            ComboBox {
                textRole: "label"
                valueRole: "value"
                currentValue: root.pageController.sortMode
                model: [
                    { label: qsTranslate("WorldListPage", "Name"), value: "name" },
                    { label: qsTranslate("WorldListPage", "Game mode"), value: "gameMode" },
                    { label: qsTranslate("WorldListPage", "Last played"), value: "lastPlayed" }
                ]
                onActivated: root.pageController.sortMode = currentValue
            }

            Button {
                text: root.pageController.sortDescending
                      ? qsTranslate("WorldListPage", "Descending")
                      : qsTranslate("WorldListPage", "Ascending")
                onClicked: root.pageController.sortDescending =
                               !root.pageController.sortDescending
            }
        }

        TextField {
            id: filterField
            Layout.fillWidth: true
            placeholderText: qsTranslate("WorldListPage", "Filter worlds...")
            onTextChanged: root.pageController.filterText = text
        }

        ListView {
            id: worldList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 7
            model: root.pageController.worlds

            delegate: ItemDelegate {
                id: worldDelegate
                required property var modelData
                width: worldList.width
                height: 84
                highlighted: modelData.selected

                background: Rectangle {
                    radius: 8
                    color: worldDelegate.highlighted
                           ? Qt.darker(palette.highlight, 1.15)
                           : (worldDelegate.hovered
                              ? palette.alternateBase : palette.base)
                    border.color: worldDelegate.highlighted
                                  ? palette.highlight : palette.mid
                }

                contentItem: RowLayout {
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 56
                        Layout.preferredHeight: 56
                        radius: 6
                        color: palette.alternateBase

                        Image {
                            anchors.fill: parent
                            anchors.margins: 2
                            visible: worldDelegate.modelData.hasIcon
                            source: worldDelegate.modelData.iconUrl
                            asynchronous: true
                            fillMode: Image.PreserveAspectFit
                        }

                        Label {
                            anchors.centerIn: parent
                            visible: !worldDelegate.modelData.hasIcon
                            text: "W"
                            color: palette.windowText
                            font.pixelSize: 24
                            font.bold: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 3

                        Text {
                            Layout.fillWidth: true
                            text: worldDelegate.modelData.name
                            textFormat: Text.PlainText
                            color: palette.windowText
                            font.pixelSize: 14
                            font.bold: worldDelegate.modelData.selected
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: [worldDelegate.modelData.gameMode,
                                   worldDelegate.modelData.lastPlayed]
                                  .filter(function(value) { return value.length > 0 })
                                  .join("  ·  ")
                            textFormat: Text.PlainText
                            color: palette.windowText
                            opacity: 0.68
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }
                }

                TapHandler {
                    onTapped: root.pageController.selectWorld(
                                  worldDelegate.modelData.folderName)
                    onDoubleTapped: {
                        root.pageController.selectWorld(
                            worldDelegate.modelData.folderName)
                        if (root.pageController.actionStates.join)
                            root.pageController.triggerAction("join")
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: worldList.count === 0
                text: filterField.text.length > 0
                      ? qsTranslate("WorldListPage", "No worlds match this filter.")
                      : qsTranslate("WorldListPage", "No worlds found.")
                color: palette.windowText
                opacity: 0.65
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: selectedDetails.implicitHeight + 24
            radius: 8
            color: palette.base
            border.color: palette.mid
            visible: root.pageController.selectedWorld.folderName !== undefined

            ColumnLayout {
                id: selectedDetails
                anchors.fill: parent
                anchors.margins: 12
                spacing: 4

                Text {
                    Layout.fillWidth: true
                    text: root.pageController.selectedWorld.name
                    textFormat: Text.PlainText
                    color: palette.windowText
                    font.pixelSize: 14
                    font.bold: true
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTranslate("WorldListPage", "Seed: %1")
                          .arg(root.pageController.selectedWorld.seed)
                    textFormat: Text.PlainText
                    color: palette.windowText
                    opacity: 0.78
                    font.pixelSize: 12
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { label: qsTranslate("WorldListPage", "Add world"), action: "add" },
                    { label: qsTranslate("WorldListPage", "Join"), action: "join" },
                    { label: qsTranslate("WorldListPage", "Join offline"), action: "joinOffline" },
                    { label: qsTranslate("WorldListPage", "Rename"), action: "rename" },
                    { label: qsTranslate("WorldListPage", "Copy"), action: "copy" },
                    { label: qsTranslate("WorldListPage", "Delete"), action: "remove" },
                    { label: qsTranslate("WorldListPage", "Open in MCEdit"), action: "mcedit" },
                    { label: qsTranslate("WorldListPage", "Open datapacks"), action: "datapacks" },
                    { label: qsTranslate("WorldListPage", "Reset icon"), action: "resetIcon" },
                    { label: qsTranslate("WorldListPage", "Copy seed"), action: "copySeed" },
                    { label: qsTranslate("WorldListPage", "Refresh"), action: "refresh" },
                    { label: qsTranslate("WorldListPage", "Open folder"), action: "folder" }
                ]

                delegate: Button {
                    required property var modelData
                    text: modelData.label
                    visible: (modelData.action !== "join" &&
                              modelData.action !== "joinOffline") ||
                             root.pageController.actionStates.quickPlayAvailable
                    enabled: root.pageController.actionStates[modelData.action]
                    onClicked: root.pageController.triggerAction(modelData.action)
                }
            }
        }
    }
}
