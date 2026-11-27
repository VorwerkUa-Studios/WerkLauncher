import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var dialogController

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    Rectangle {
        anchors.fill: parent
        color: palette.window

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            Text {
                text: qsTranslate("ModrinthExportDialog", "Export Modrinth modpack")
                color: palette.windowText
                font.pixelSize: 18
                font.bold: true
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 14

                    // Metadata Card
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: metaCol.implicitHeight + 24
                        radius: 10
                        color: palette.base
                        border.color: palette.mid

                        ColumnLayout {
                            id: metaCol
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10

                            Text {
                                text: qsTranslate("ModrinthExportDialog", "Metadata")
                                color: palette.windowText
                                font.bold: true
                                font.pixelSize: 13
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 2
                                    spacing: 4

                                    Text {
                                        text: qsTranslate("ModrinthExportDialog", "Name:")
                                        color: palette.windowText
                                        font.pixelSize: 11
                                        opacity: 0.8
                                    }

                                    TextField {
                                        Layout.fillWidth: true
                                        text: root.dialogController.name
                                        placeholderText: qsTranslate("ModrinthExportDialog", "Modpack name")
                                        onTextChanged: root.dialogController.name = text
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 1
                                    spacing: 4

                                    Text {
                                        text: qsTranslate("ModrinthExportDialog", "Version:")
                                        color: palette.windowText
                                        font.pixelSize: 11
                                        opacity: 0.8
                                    }

                                    TextField {
                                        Layout.fillWidth: true
                                        text: root.dialogController.version
                                        placeholderText: "1.0"
                                        onTextChanged: root.dialogController.version = text
                                    }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Text {
                                    text: qsTranslate("ModrinthExportDialog", "Description:")
                                    color: palette.windowText
                                    font.pixelSize: 11
                                    opacity: 0.8
                                }

                                TextField {
                                    Layout.fillWidth: true
                                    text: root.dialogController.description
                                    placeholderText: qsTranslate("ModrinthExportDialog", "Brief modpack description")
                                    onTextChanged: root.dialogController.description = text
                                }
                            }
                        }
                    }

                    // Export File Card
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: fileCol.implicitHeight + 24
                        radius: 10
                        color: palette.base
                        border.color: palette.mid

                        ColumnLayout {
                            id: fileCol
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 6

                            Text {
                                text: qsTranslate("ModrinthExportDialog", "Output File (.mrpack)")
                                color: palette.windowText
                                font.bold: true
                                font.pixelSize: 13
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                TextField {
                                    Layout.fillWidth: true
                                    text: root.dialogController.exportPath
                                    placeholderText: qsTranslate("ModrinthExportDialog", "Path to export .mrpack file")
                                    onTextChanged: root.dialogController.exportPath = text
                                }

                                Button {
                                    text: qsTranslate("ModrinthExportDialog", "Browse...")
                                    onClicked: root.dialogController.browseExportPath()
                                }
                            }
                        }
                    }

                    // Options Card
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: optCol.implicitHeight + 24
                        radius: 10
                        color: palette.base
                        border.color: palette.mid

                        ColumnLayout {
                            id: optCol
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8

                            Text {
                                text: qsTranslate("ModrinthExportDialog", "Export Options")
                                color: palette.windowText
                                font.bold: true
                                font.pixelSize: 13
                            }

                            CheckBox {
                                text: qsTranslate("ModrinthExportDialog", "Include Minecraft config")
                                checked: root.dialogController.includeGameConfig
                                onToggled: root.dialogController.includeGameConfig = checked
                            }

                            CheckBox {
                                text: qsTranslate("ModrinthExportDialog", "Include mod configs")
                                checked: root.dialogController.includeModConfigs
                                onToggled: root.dialogController.includeModConfigs = checked
                            }

                            CheckBox {
                                text: qsTranslate("ModrinthExportDialog", "Include resource packs")
                                checked: root.dialogController.includeResourcePacks
                                onToggled: root.dialogController.includeResourcePacks = checked
                            }

                            CheckBox {
                                text: qsTranslate("ModrinthExportDialog", "Include shader packs")
                                checked: root.dialogController.includeShaderPacks
                                onToggled: root.dialogController.includeShaderPacks = checked
                            }

                            CheckBox {
                                text: qsTranslate("ModrinthExportDialog", "Treat disabled mods, shaders and resource packs as optional")
                                checked: root.dialogController.treatDisabledAsOptional
                                onToggled: root.dialogController.treatDisabledAsOptional = checked
                            }

                            CheckBox {
                                id: datapacksCb
                                text: qsTranslate("ModrinthExportDialog", "Include global datapacks folder")
                                checked: root.dialogController.includeDatapacks
                                onToggled: root.dialogController.includeDatapacks = checked
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.leftMargin: 24
                                visible: datapacksCb.checked
                                spacing: 8

                                TextField {
                                    Layout.fillWidth: true
                                    text: root.dialogController.datapacksPath
                                    placeholderText: qsTranslate("ModrinthExportDialog", "Relative folder path")
                                    onTextChanged: root.dialogController.datapacksPath = text
                                }

                                Button {
                                    text: qsTranslate("ModrinthExportDialog", "Browse...")
                                    onClicked: root.dialogController.browseDatapacksPath()
                                }
                            }
                        }
                    }
                }
            }

            // Buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTranslate("ModrinthExportDialog", "Cancel")
                    onClicked: root.dialogController.rejectDialog()
                }

                Button {
                    text: qsTranslate("ModrinthExportDialog", "Export")
                    highlighted: true
                    enabled: root.dialogController.canAccept
                    onClicked: root.dialogController.acceptDialog()
                }
            }
        }
    }
}
