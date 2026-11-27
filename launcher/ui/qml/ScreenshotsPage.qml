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
                    text: qsTranslate("ScreenshotsPage", "%1 screenshots")
                          .arg(root.pageController.screenshots.length)
                    color: palette.windowText
                    opacity: 0.7
                    font.pixelSize: 13
                }
            }

            Label {
                text: root.pageController.selectedCount > 0
                      ? qsTranslate("ScreenshotsPage", "%1 selected")
                        .arg(root.pageController.selectedCount)
                      : qsTranslate("ScreenshotsPage", "Select screenshots to use actions")
                color: palette.windowText
                opacity: 0.7
            }
        }

        GridView {
            id: screenshotGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: 204
            cellHeight: 184
            model: root.pageController.screenshots

            Keys.onPressed: function(event) {
                if ((event.modifiers & Qt.ControlModifier) &&
                    event.key === Qt.Key_C) {
                    root.pageController.triggerAction("copyFiles")
                    event.accepted = true
                } else if (event.key === Qt.Key_Delete) {
                    root.pageController.triggerAction("delete")
                    event.accepted = true
                } else if (event.key === Qt.Key_F2) {
                    root.pageController.triggerAction("rename")
                    event.accepted = true
                }
            }

            delegate: ItemDelegate {
                id: screenshotDelegate
                required property var modelData
                width: screenshotGrid.cellWidth - 12
                height: screenshotGrid.cellHeight - 12
                highlighted: modelData.selected

                background: Rectangle {
                    radius: 8
                    color: screenshotDelegate.highlighted
                           ? Qt.darker(palette.highlight, 1.15)
                           : (screenshotDelegate.hovered
                              ? palette.alternateBase : palette.base)
                    border.color: screenshotDelegate.highlighted
                                  ? palette.highlight : palette.mid
                }

                contentItem: ColumnLayout {
                    spacing: 6

                    Image {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        source: screenshotDelegate.modelData.imageUrl
                        sourceSize.width: 256
                        sourceSize.height: 144
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        cache: false
                    }

                    Text {
                        Layout.fillWidth: true
                        text: screenshotDelegate.modelData.name
                        textFormat: Text.PlainText
                        color: palette.windowText
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    acceptedModifiers: Qt.NoModifier
                    onTapped: {
                        screenshotGrid.forceActiveFocus()
                        root.pageController.selectScreenshot(
                            screenshotDelegate.modelData.path, Qt.NoModifier)
                    }
                    onDoubleTapped: root.pageController.openScreenshot(
                                        screenshotDelegate.modelData.path)
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    acceptedModifiers: Qt.ControlModifier
                    onTapped: {
                        screenshotGrid.forceActiveFocus()
                        root.pageController.selectScreenshot(
                            screenshotDelegate.modelData.path, Qt.ControlModifier)
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    acceptedModifiers: Qt.ShiftModifier
                    onTapped: {
                        screenshotGrid.forceActiveFocus()
                        root.pageController.selectScreenshot(
                            screenshotDelegate.modelData.path, Qt.ShiftModifier)
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    acceptedModifiers: Qt.ControlModifier | Qt.ShiftModifier
                    onTapped: {
                        screenshotGrid.forceActiveFocus()
                        root.pageController.selectScreenshot(
                            screenshotDelegate.modelData.path,
                            Qt.ControlModifier | Qt.ShiftModifier)
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: screenshotGrid.count === 0
                text: qsTranslate("ScreenshotsPage", "No screenshots found.")
                color: palette.windowText
                opacity: 0.65
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { label: qsTranslate("ScreenshotsPage", "Upload"), action: "upload" },
                    { label: qsTranslate("ScreenshotsPage", "Copy image"), action: "copyImage" },
                    { label: qsTranslate("ScreenshotsPage", "Copy files"), action: "copyFiles" },
                    { label: qsTranslate("ScreenshotsPage", "Delete"), action: "delete" },
                    { label: qsTranslate("ScreenshotsPage", "Rename"), action: "rename" },
                    { label: qsTranslate("ScreenshotsPage", "Open folder"), action: "folder" }
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
