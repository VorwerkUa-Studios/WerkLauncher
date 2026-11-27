pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var versionController

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                Text {
                    text: qsTranslate("VersionPage", "Version components")
                    color: palette.windowText
                    font.pixelSize: 22
                    font.bold: true
                }

                Text {
                    text: qsTranslate("VersionPage",
                                      "Review and manage the components used by this instance.")
                    color: palette.windowText
                    opacity: 0.72
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }

            Button {
                text: qsTranslate("VersionPage", "Download all")
                enabled: root.versionController.actionStates.downloadAll
                onClicked: root.versionController.triggerAction("actionDownload_All")
            }

            Button {
                text: qsTranslate("VersionPage", "Reload")
                enabled: root.versionController.actionStates.reload
                onClicked: root.versionController.triggerAction("actionReload")
            }
        }

        TextField {
            id: filterField
            Layout.fillWidth: true
            placeholderText: qsTranslate("VersionPage", "Filter components...")
            onTextChanged: root.versionController.filterText = text
        }

        ListView {
            id: componentList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: root.versionController.components

            delegate: ItemDelegate {
                id: componentDelegate
                required property var modelData
                width: componentList.width
                height: 66
                highlighted: modelData.sourceRow
                             === root.versionController.selectedComponentIndex
                onClicked: root.versionController.selectComponent(modelData.sourceRow)

                background: Rectangle {
                    radius: 8
                    color: componentDelegate.highlighted
                           ? Qt.darker(palette.highlight, 1.15)
                           : (componentDelegate.hovered
                              ? palette.alternateBase : palette.base)
                    border.color: componentDelegate.highlighted
                                  ? palette.highlight : palette.mid
                }

                contentItem: RowLayout {
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 10
                        Layout.preferredHeight: 10
                        radius: 5
                        color: componentDelegate.modelData.severity === "error" ? "#e05252"
                               : componentDelegate.modelData.severity === "warning" ? "#e0a33a"
                               : "#5caa6a"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            Layout.fillWidth: true
                            text: componentDelegate.modelData.name
                            color: palette.windowText
                            font.pixelSize: 14
                            font.bold: componentDelegate.highlighted
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: componentDelegate.modelData.version
                            color: palette.windowText
                            opacity: 0.68
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }

                    CheckBox {
                        visible: componentDelegate.modelData.canBeDisabled
                        enabled: root.versionController.actionStates.reload
                        checked: componentDelegate.modelData.enabled
                        text: qsTranslate("VersionPage", "Enabled")
                        onToggled: root.versionController.setComponentEnabled(
                                       componentDelegate.modelData.sourceRow, checked)
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: componentList.count === 0
                text: qsTranslate("VersionPage", "No components match this filter.")
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
            visible: root.versionController.selectedComponent.name !== undefined

            ColumnLayout {
                id: selectedDetails
                anchors.fill: parent
                anchors.margins: 12
                spacing: 5

                Text {
                    Layout.fillWidth: true
                    text: root.versionController.selectedComponent.summary
                          || root.versionController.selectedComponent.name
                    color: palette.windowText
                    font.pixelSize: 14
                    font.bold: true
                    wrapMode: Text.WordWrap
                }

                Text {
                    Layout.fillWidth: true
                    text: root.versionController.selectedComponent.problems || ""
                    visible: text.length > 0
                    color: palette.windowText
                    opacity: 0.78
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { label: qsTranslate("VersionPage", "Change version"), state: "changeVersion", action: "actionChange_version" },
                    { label: qsTranslate("VersionPage", "Move up"), state: "moveUp", action: "actionMove_up" },
                    { label: qsTranslate("VersionPage", "Move down"), state: "moveDown", action: "actionMove_down" },
                    { label: qsTranslate("VersionPage", "Remove"), state: "remove", action: "actionRemove" },
                    { label: qsTranslate("VersionPage", "Customize"), state: "customize", action: "actionCustomize" },
                    { label: qsTranslate("VersionPage", "Edit"), state: "edit", action: "actionEdit" },
                    { label: qsTranslate("VersionPage", "Revert"), state: "revert", action: "actionRevert" }
                ]

                delegate: Button {
                    required property var modelData
                    text: modelData.label
                    enabled: root.versionController.actionStates[modelData.state]
                    onClicked: root.versionController.triggerAction(modelData.action)
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { label: qsTranslate("VersionPage", "Install Forge"), state: "installForge", action: "actionInstall_Forge" },
                    { label: qsTranslate("VersionPage", "Install NeoForge"), state: "installNeoForge", action: "actionInstall_NeoForge" },
                    { label: qsTranslate("VersionPage", "Install Fabric"), state: "installFabric", action: "actionInstall_Fabric" },
                    { label: qsTranslate("VersionPage", "Install Quilt"), state: "installQuilt", action: "actionInstall_Quilt" },
                    { label: qsTranslate("VersionPage", "Install LiteLoader"), state: "installLiteLoader", action: "actionInstall_LiteLoader" },
                    { label: qsTranslate("VersionPage", "Install mods"), state: "installMods", action: "actionInstall_mods" }
                ]

                delegate: Button {
                    required property var modelData
                    text: modelData.label
                    enabled: root.versionController.actionStates[modelData.state]
                    onClicked: root.versionController.triggerAction(modelData.action)
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { label: qsTranslate("VersionPage", "Add jar mods"), state: "addJarMods", action: "actionAdd_to_Minecraft_jar" },
                    { label: qsTranslate("VersionPage", "Replace Minecraft jar"), state: "replaceMinecraftJar", action: "actionReplace_Minecraft_jar" },
                    { label: qsTranslate("VersionPage", "Add empty component"), state: "addEmpty", action: "actionAdd_Empty" },
                    { label: qsTranslate("VersionPage", "Open game folder"), state: "openMinecraftFolder", action: "actionMinecraftFolder" },
                    { label: qsTranslate("VersionPage", "Open libraries"), state: "openLibrariesFolder", action: "actionLibrariesFolder" }
                ]

                delegate: Button {
                    required property var modelData
                    text: modelData.label
                    enabled: root.versionController.actionStates[modelData.state]
                    onClicked: root.versionController.triggerAction(modelData.action)
                }
            }
        }
    }
}
