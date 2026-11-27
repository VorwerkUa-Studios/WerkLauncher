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
            anchors.margins: 18
            spacing: 16

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16

                Text {
                    text: root.dialogController.notificationType === "critical" ? "⚠️" : (root.dialogController.notificationType === "warning" ? "⚡" : "ℹ️")
                    font.pixelSize: 36
                    verticalAlignment: Text.AlignVCenter
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: root.dialogController.message
                    color: palette.windowText
                    wrapMode: Text.WordWrap
                    font.pixelSize: 13
                    lineHeight: 1.3
                    verticalAlignment: Text.AlignVCenter
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: root.dialogController.dontShowAgainCountdown > 0 ? qsTranslate("NotificationDialog", "Don't show again") + " (" + root.dialogController.dontShowAgainCountdown + ")" : qsTranslate("NotificationDialog", "Don't show again")
                    enabled: root.dialogController.dontShowAgainCountdown === 0
                    onClicked: root.dialogController.dontShowAgain()
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: root.dialogController.closeCountdown > 0 ? qsTranslate("NotificationDialog", "Close") + " (" + root.dialogController.closeCountdown + ")" : qsTranslate("NotificationDialog", "Close")
                    enabled: root.dialogController.closeCountdown === 0
                    highlighted: true
                    onClicked: root.dialogController.dismiss()
                }
            }
        }
    }
}
