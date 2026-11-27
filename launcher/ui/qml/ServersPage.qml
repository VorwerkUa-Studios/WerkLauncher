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
                    text: qsTranslate("ServersPage", "%1 saved servers")
                          .arg(root.pageController.servers.length)
                    color: palette.windowText
                    opacity: 0.7
                    font.pixelSize: 13
                }
            }

            Label {
                text: qsTranslate("ServersPage", "Changes save automatically")
                color: palette.windowText
                opacity: 0.65
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 14

            ListView {
                id: serverList
                Layout.preferredWidth: Math.max(260, parent.width * 0.36)
                Layout.fillHeight: true
                clip: true
                spacing: 6
                model: root.pageController.servers

                delegate: ItemDelegate {
                    id: serverDelegate
                    required property var modelData
                    width: serverList.width
                    height: 74
                    highlighted: modelData.selected

                    background: Rectangle {
                        radius: 8
                        color: serverDelegate.highlighted
                               ? Qt.darker(palette.highlight, 1.15)
                               : (serverDelegate.hovered
                                  ? palette.alternateBase : palette.base)
                        border.color: serverDelegate.highlighted
                                      ? palette.highlight : palette.mid
                    }

                    contentItem: RowLayout {
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            radius: 6
                            color: palette.alternateBase

                            Image {
                                anchors.fill: parent
                                anchors.margins: 2
                                visible: serverDelegate.modelData.iconUrl.length > 0
                                source: serverDelegate.modelData.iconUrl
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                            }

                            Label {
                                anchors.centerIn: parent
                                visible: !serverDelegate.modelData.iconUrl.length
                                text: "S"
                                color: palette.windowText
                                font.pixelSize: 22
                                font.bold: true
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3

                            Text {
                                Layout.fillWidth: true
                                text: serverDelegate.modelData.name
                                textFormat: Text.PlainText
                                color: palette.windowText
                                font.pixelSize: 14
                                font.bold: serverDelegate.modelData.selected
                                elide: Text.ElideRight
                            }

                            Text {
                                Layout.fillWidth: true
                                text: serverDelegate.modelData.address
                                textFormat: Text.PlainText
                                color: palette.windowText
                                opacity: 0.68
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }
                        }
                    }

                    onClicked: root.pageController.selectServer(modelData.row)
                }

                Label {
                    anchors.centerIn: parent
                    visible: serverList.count === 0
                    text: qsTranslate("ServersPage", "No servers saved yet.")
                    color: palette.windowText
                    opacity: 0.65
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: palette.base
                border.color: palette.mid

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12
                    enabled: root.pageController.actionStates.edit

                    Text {
                        text: qsTranslate("ServersPage", "Server details")
                        color: palette.windowText
                        font.pixelSize: 17
                        font.bold: true
                    }

                    Label {
                        text: qsTranslate("ServersPage", "Name")
                    }

                    TextField {
                        Layout.fillWidth: true
                        text: root.pageController.selectedServer.name || ""
                        placeholderText: qsTranslate("ServersPage", "Minecraft Server")
                        onTextEdited: root.pageController.setServerName(text)
                    }

                    Label {
                        text: qsTranslate("ServersPage", "Address")
                    }

                    TextField {
                        Layout.fillWidth: true
                        text: root.pageController.selectedServer.address || ""
                        placeholderText: qsTranslate("ServersPage", "host:port")
                        onTextEdited: root.pageController.setServerAddress(text)
                    }

                    Label {
                        text: qsTranslate("ServersPage", "Server resource packs")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                        currentIndex: root.pageController.selectedServer.acceptsTextures || 0
                        model: [
                            qsTranslate("ServersPage", "Ask"),
                            qsTranslate("ServersPage", "Always accept"),
                            qsTranslate("ServersPage", "Never accept")
                        ]
                        onActivated: root.pageController.setAcceptsTextures(currentIndex)
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { label: qsTranslate("ServersPage", "Add server"), action: "add" },
                    { label: qsTranslate("ServersPage", "Remove"), action: "remove" },
                    { label: qsTranslate("ServersPage", "Move up"), action: "moveUp" },
                    { label: qsTranslate("ServersPage", "Move down"), action: "moveDown" },
                    { label: qsTranslate("ServersPage", "Join"), action: "join" }
                ]

                delegate: Button {
                    required property var modelData
                    text: modelData.label
                    enabled: root.pageController.actionStates[modelData.action]
                    onClicked: root.pageController.triggerAction(modelData.action)
                }
            }
        }
    }
}
