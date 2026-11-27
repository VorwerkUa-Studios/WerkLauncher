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

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Image {
                    source: "image://themedicon/checkupdate"
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    Layout.fillWidth: true
                    text: root.dialogController.headerText
                    color: palette.windowText
                    font.pixelSize: 16
                    font.bold: true
                    wrapMode: Text.WordWrap
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: palette.base
                border.color: palette.mid
                clip: true

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 14
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    Text {
                        width: parent.width
                        text: root.dialogController.changelogHtml
                        textFormat: Text.RichText
                        color: palette.windowText
                        wrapMode: Text.WordWrap
                        font.pixelSize: 13
                        onLinkActivated: function(link) { Qt.openUrlExternally(link) }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Item { Layout.fillWidth: true }

                Button {
                    text: root.dialogController.hasUpdate ? qsTranslate("UpdateDialog", "Update Later") : qsTranslate("UpdateDialog", "Close")
                    onClicked: root.dialogController.updateLater()
                }

                Button {
                    visible: root.dialogController.hasUpdate
                    text: qsTranslate("UpdateDialog", "Update Now")
                    highlighted: true
                    onClicked: root.dialogController.updateNow()
                }
            }
        }
    }
}
