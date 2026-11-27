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
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: root.dialogController.statusText || qsTranslate("ProgressDialog", "Please wait...")
                color: palette.windowText
                font.pixelSize: 13
                elide: Text.ElideRight
            }

            ProgressBar {
                Layout.fillWidth: true
                Layout.preferredHeight: 8
                value: root.dialogController.progressValue
                from: 0
                to: root.dialogController.progressMaximum > 0 ? root.dialogController.progressMaximum : 100
                indeterminate: root.dialogController.isIndeterminate
            }

            RowLayout {
                Layout.fillWidth: true
                visible: root.dialogController.skipButtonVisible

                Item { Layout.fillWidth: true }

                Button {
                    text: root.dialogController.skipButtonText || qsTranslate("ProgressDialog", "Abort")
                    onClicked: root.dialogController.abortTask()
                }
            }
        }
    }
}
