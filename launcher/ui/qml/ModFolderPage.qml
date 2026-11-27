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
                    color: palette.windowText
                    font.pixelSize: 22
                    font.bold: true
                }

                Text {
                    text: qsTranslate("ModFolderPage", "%1 items")
                          .arg(root.pageController.items.length)
                    color: palette.windowText
                    opacity: 0.7
                    font.pixelSize: 13
                }
            }

            Label {
                text: qsTranslate("ModFolderPage", "Sort by")
            }

            ComboBox {
                id: sortMode
                textRole: "label"
                valueRole: "value"
                currentValue: root.pageController.sortMode
                model: [
                    { label: qsTranslate("ModFolderPage", "Name"), value: "name" },
                    { label: qsTranslate("ModFolderPage", "Version"), value: "version" },
                    { label: qsTranslate("ModFolderPage", "Last changed"), value: "modified" },
                    { label: qsTranslate("ModFolderPage", "Enabled state"), value: "enabled" }
                ]
                onActivated: root.pageController.sortMode = currentValue
            }

            Button {
                text: root.pageController.sortDescending
                      ? qsTranslate("ModFolderPage", "Descending")
                      : qsTranslate("ModFolderPage", "Ascending")
                onClicked: root.pageController.sortDescending =
                               !root.pageController.sortDescending
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            TextField {
                id: filterField
                Layout.fillWidth: true
                placeholderText: qsTranslate("ModFolderPage", "Filter by name, description, or author...")
                onTextChanged: root.pageController.filterText = text
            }

            Button {
                text: qsTranslate("ModFolderPage", "Clear")
                enabled: filterField.text.length > 0
                onClicked: filterField.clear()
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ListView {
                id: modList
                anchors.fill: parent
                clip: true
                spacing: 6
                focus: true
                model: root.pageController.items

                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Delete) {
                        root.pageController.triggerAction("remove")
                        event.accepted = true
                    } else if (event.key === Qt.Key_Plus) {
                        root.pageController.triggerAction("add")
                        event.accepted = true
                    }
                }

                delegate: ItemDelegate {
                    id: modDelegate
                    required property var modelData
                    width: modList.width
                    height: 92
                    highlighted: modelData.selected

                    background: Rectangle {
                        radius: 8
                        color: modDelegate.highlighted
                               ? Qt.darker(palette.highlight, 1.15)
                               : (modDelegate.hovered
                                  ? palette.alternateBase : palette.base)
                        border.color: modDelegate.highlighted
                                      ? palette.highlight : palette.mid
                    }

                    contentItem: RowLayout {
                        spacing: 12

                        CheckBox {
                            Accessible.name: qsTranslate("ModFolderPage", "Enable %1")
                                                    .arg(modDelegate.modelData.name)
                            enabled: root.pageController.actionStates.toggle
                            checked: modDelegate.modelData.enabled
                            onToggled: root.pageController.setModEnabled(
                                           modDelegate.modelData.sourceRow, checked)
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                Layout.fillWidth: true
                                text: modDelegate.modelData.name
                                textFormat: Text.PlainText
                                color: palette.windowText
                                font.pixelSize: 14
                                font.bold: modDelegate.modelData.selected
                                elide: Text.ElideRight
                            }

                            Text {
                                Layout.fillWidth: true
                                textFormat: Text.PlainText
                                text: {
                                    const details = [modDelegate.modelData.version]
                                    if (modDelegate.modelData.authors.length > 0)
                                        details.push(modDelegate.modelData.authors)
                                    if (modDelegate.modelData.modified.length > 0)
                                        details.push(modDelegate.modelData.modified)
                                    return details.join("  ·  ")
                                }
                                color: palette.windowText
                                opacity: 0.68
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }

                            Text {
                                Layout.fillWidth: true
                                text: modDelegate.modelData.description
                                textFormat: Text.PlainText
                                color: palette.windowText
                                opacity: 0.72
                                font.pixelSize: 12
                                elide: Text.ElideRight
                                maximumLineCount: 1
                            }
                        }

                        Label {
                            visible: modDelegate.modelData.hasUpdate
                            text: qsTranslate("ModFolderPage", "Update available")
                            color: "#b87800"
                        }

                        Button {
                            visible: modDelegate.modelData.hasUpdate
                            text: qsTranslate("ModFolderPage", "Update")
                            onClicked: root.pageController.installUpdate(
                                           modDelegate.modelData.sourceRow)
                        }
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        acceptedModifiers: Qt.NoModifier
                        onTapped: {
                            modList.forceActiveFocus()
                            root.pageController.selectMod(
                                modDelegate.modelData.id, Qt.NoModifier)
                        }
                        onDoubleTapped: root.pageController.setModEnabled(
                                            modDelegate.modelData.sourceRow,
                                            !modDelegate.modelData.enabled)
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        acceptedModifiers: Qt.ControlModifier
                        onTapped: {
                            modList.forceActiveFocus()
                            root.pageController.selectMod(
                                modDelegate.modelData.id, Qt.ControlModifier)
                        }
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        acceptedModifiers: Qt.ShiftModifier
                        onTapped: {
                            modList.forceActiveFocus()
                            root.pageController.selectMod(
                                modDelegate.modelData.id, Qt.ShiftModifier)
                        }
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        acceptedModifiers: Qt.ControlModifier | Qt.ShiftModifier
                        onTapped: {
                            modList.forceActiveFocus()
                            root.pageController.selectMod(
                                modDelegate.modelData.id,
                                Qt.ControlModifier | Qt.ShiftModifier)
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: modList.count === 0
                    text: filterField.text.length > 0
                          ? qsTranslate("ModFolderPage", "No items match this filter.")
                          : qsTranslate("ModFolderPage", "This folder is empty.")
                    color: palette.windowText
                    opacity: 0.65
                }
            }

            DropArea {
                anchors.fill: parent
                onDropped: function(drop) {
                    root.pageController.installDroppedFiles(drop.urls)
                    drop.acceptProposedAction()
                }

                Rectangle {
                    anchors.fill: parent
                    visible: parent.containsDrag
                    color: Qt.rgba(palette.highlight.r, palette.highlight.g,
                                   palette.highlight.b, 0.12)
                    border.color: palette.highlight
                    border.width: 2
                    radius: 8
                    Label {
                        anchors.centerIn: parent
                        text: qsTranslate("ModFolderPage", "Drop files here to add them")
                        color: palette.windowText
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: detailsColumn.implicitHeight + 24
            radius: 8
            color: palette.base
            border.color: palette.mid
            visible: root.pageController.selectedMod.id !== undefined

            ColumnLayout {
                id: detailsColumn
                anchors.fill: parent
                anchors.margins: 12
                spacing: 5

                Text {
                    Layout.fillWidth: true
                    text: root.pageController.selectedMod.name
                    textFormat: Text.PlainText
                    color: palette.windowText
                    font.pixelSize: 14
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: root.pageController.selectedMod.description || ""
                    textFormat: Text.PlainText
                    visible: text.length > 0
                    color: palette.windowText
                    opacity: 0.78
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    maximumLineCount: 3
                    elide: Text.ElideRight
                }

                Button {
                    Layout.alignment: Qt.AlignLeft
                    visible: String(root.pageController.selectedMod.homepage || "").length > 0
                    text: qsTranslate("ModFolderPage", "Open project page")
                    onClicked: root.pageController.openModHomepage()
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { label: qsTranslate("ModFolderPage", "Add files"), action: "add" },
                    { label: qsTranslate("ModFolderPage", "Download"), action: "download" },
                    { label: qsTranslate("ModFolderPage", "Update all"), action: "updateAll" },
                    { label: qsTranslate("ModFolderPage", "Remove selected"), action: "remove" },
                    { label: qsTranslate("ModFolderPage", "Enable selected"), action: "enable" },
                    { label: qsTranslate("ModFolderPage", "Disable selected"), action: "disable" },
                    { label: qsTranslate("ModFolderPage", "View configs"), action: "configs" },
                    { label: qsTranslate("ModFolderPage", "Open folder"), action: "folder" }
                ]

                delegate: Button {
                    required property var modelData
                    text: modelData.label
                    visible: modelData.action !== "configs"
                             || root.pageController.actionStates.configs
                    enabled: root.pageController.actionStates[modelData.action]
                    onClicked: root.pageController.triggerAction(modelData.action)
                }
            }
        }
    }
}
