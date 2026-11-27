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
        anchors.margins: 12
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 40
            color: palette.alternateBase
            border.color: palette.mid

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 16

                Text {
                    Layout.preferredWidth: parent.width * 0.35
                    text: qsTranslate("GameOptionsPage", "Key")
                    color: palette.windowText
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTranslate("GameOptionsPage", "Value")
                    color: palette.windowText
                    font.bold: true
                }
            }
        }

        ListView {
            id: optionsList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.pageController.options
            boundsBehavior: Flickable.StopAtBounds

            delegate: Rectangle {
                id: optionDelegate
                required property int index
                required property var modelData
                width: optionsList.width
                implicitHeight: Math.max(keyText.implicitHeight,
                                         valueText.implicitHeight) + 18
                color: index % 2 === 0 ? palette.base : palette.alternateBase
                border.color: palette.mid
                border.width: 0.5

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 16

                    TextEdit {
                        id: keyText
                        Layout.preferredWidth: parent.width * 0.35
                        text: optionDelegate.modelData.key
                        color: palette.windowText
                        wrapMode: TextEdit.WrapAnywhere
                        textFormat: TextEdit.PlainText
                        readOnly: true
                        selectByMouse: true
                    }

                    TextEdit {
                        id: valueText
                        Layout.fillWidth: true
                        text: optionDelegate.modelData.value
                        color: palette.windowText
                        wrapMode: TextEdit.WrapAnywhere
                        textFormat: TextEdit.PlainText
                        readOnly: true
                        selectByMouse: true
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}

            Label {
                anchors.centerIn: parent
                visible: optionsList.count === 0
                text: qsTranslate("GameOptionsPage", "No game options found.")
                color: palette.windowText
                opacity: 0.6
            }
        }
    }
}
