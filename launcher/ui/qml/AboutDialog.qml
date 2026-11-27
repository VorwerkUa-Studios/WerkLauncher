import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var dialogController

    property int currentTab: 0

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

            // Header
            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                Image {
                    source: "image://themedicon/logo"
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64
                    fillMode: Image.PreserveAspectFit
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: root.dialogController.launcherName
                        color: palette.windowText
                        font.pixelSize: 20
                        font.bold: true
                    }

                    Text {
                        text: qsTranslate("AboutDialog", "Version") + ": " + root.dialogController.versionString
                        color: palette.windowText
                        opacity: 0.8
                        font.pixelSize: 13
                    }

                    Text {
                        text: qsTranslate("AboutDialog", "Platform") + ": " + root.dialogController.platformString + (root.dialogController.channelString ? " (" + root.dialogController.channelString + ")" : "")
                        color: palette.windowText
                        opacity: 0.6
                        font.pixelSize: 12
                    }
                }
            }

            // Tab bar
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                TabButton {
                    Layout.fillWidth: true
                    text: qsTranslate("AboutDialog", "About")
                    active: root.currentTab === 0
                    onClicked: root.currentTab = 0
                }

                TabButton {
                    Layout.fillWidth: true
                    text: qsTranslate("AboutDialog", "Credits")
                    active: root.currentTab === 1
                    onClicked: root.currentTab = 1
                }

                TabButton {
                    Layout.fillWidth: true
                    text: qsTranslate("AboutDialog", "License")
                    active: root.currentTab === 2
                    onClicked: root.currentTab = 2
                }
            }

            // Tab content
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: palette.base
                border.color: palette.mid
                clip: true

                // About tab
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 16
                    visible: root.currentTab === 0
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    ColumnLayout {
                        width: parent.width
                        spacing: 12

                        Text {
                            Layout.fillWidth: true
                            text: qsTranslate("AboutDialog", "WerkLauncher is a free, open source Minecraft launcher with multiple instances management, custom themes and enhanced mod support.")
                            color: palette.windowText
                            wrapMode: Text.WordWrap
                            font.pixelSize: 13
                            lineHeight: 1.3
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "<a href='" + root.dialogController.urlString + "'>" + root.dialogController.urlString + "</a>"
                            color: palette.highlight
                            linkColor: palette.highlight
                            onLinkActivated: function(link) { Qt.openUrlExternally(link) }
                            font.pixelSize: 13
                        }
                    }
                }

                // Credits tab
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 16
                    visible: root.currentTab === 1
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    Text {
                        width: parent.width
                        text: root.dialogController.creditsHtml
                        textFormat: Text.RichText
                        color: palette.windowText
                        wrapMode: Text.WordWrap
                        onLinkActivated: function(link) { Qt.openUrlExternally(link) }
                    }
                }

                // License tab
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 16
                    visible: root.currentTab === 2
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    Text {
                        width: parent.width
                        text: root.dialogController.licenseHtml
                        textFormat: Text.RichText
                        color: palette.windowText
                        wrapMode: Text.WordWrap
                        font.pixelSize: 12
                    }
                }
            }

            // Footer buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "About Qt"
                    onClicked: root.dialogController.showAboutQt()
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTranslate("AboutDialog", "Close")
                    highlighted: true
                    onClicked: root.dialogController.closeDialog()
                }
            }
        }
    }

    component TabButton: Rectangle {
        id: tabBtn
        required property string text
        required property bool active
        signal clicked()

        implicitHeight: 34
        radius: 6
        color: active ? palette.highlight : (tabMouse.containsMouse ? Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.25) : "transparent")
        border.color: active ? palette.highlight : palette.mid
        border.width: 1

        MouseArea {
            id: tabMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: tabBtn.clicked()
        }

        Text {
            anchors.centerIn: parent
            text: tabBtn.text
            color: tabBtn.active ? palette.highlightedText : palette.windowText
            font.bold: tabBtn.active
            font.pixelSize: 13
        }
    }
}
