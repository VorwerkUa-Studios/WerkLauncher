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

    function findInFile(reverse) {
        const query = searchField.text
        if (query.length === 0)
            return

        let start = reverse ? logText.selectionStart - 1 : logText.selectionEnd
        const found = root.pageController.findInContent(query, start, reverse)
        if (found >= 0)
            logText.select(found, found + query.length)
    }

    function syncFileSelection() {
        fileSelector.currentIndex = root.pageController.files.indexOf(
                                        root.pageController.currentFile)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            ComboBox {
                id: fileSelector
                Layout.fillWidth: true
                model: root.pageController.files
                Component.onCompleted: root.syncFileSelection()
                onActivated: root.pageController.selectFile(currentText)

                Connections {
                    target: root.pageController
                    function onFilesChanged() {
                        root.syncFileSelection()
                    }
                    function onFileSelectionChanged() {
                        root.syncFileSelection()
                    }
                }
            }

            Button {
                text: qsTranslate("OtherLogsPage", "Reload")
                enabled: root.pageController.currentFile.length > 0
                onClicked: root.pageController.reload()
            }

            Button {
                text: qsTranslate("OtherLogsPage", "Copy")
                enabled: root.pageController.currentFile.length > 0
                onClicked: root.pageController.copyLog()
            }

            Button {
                text: qsTranslate("OtherLogsPage", "Upload")
                enabled: root.pageController.currentFile.length > 0
                onClicked: root.pageController.uploadLog()
            }

            Button {
                text: qsTranslate("OtherLogsPage", "Delete")
                enabled: root.pageController.currentFile.length > 0
                onClicked: root.pageController.deleteLog()
            }

            Button {
                text: qsTranslate("OtherLogsPage", "Clean")
                enabled: root.pageController.files.length > 0
                onClicked: root.pageController.cleanLogs()
            }
        }

        TextArea {
            id: logText
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: root.pageController.content
            font: root.pageController.textFont
            readOnly: true
            selectByMouse: true
            wrapMode: TextEdit.Wrap
            textFormat: TextEdit.PlainText
            color: palette.text
            background: Rectangle {
                color: palette.base
                border.color: palette.mid
            }

            Label {
                anchors.centerIn: parent
                visible: root.pageController.files.length === 0
                text: qsTranslate("OtherLogsPage", "No log files found.")
                color: palette.text
                opacity: 0.6
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: qsTranslate("OtherLogsPage", "Search:")
            }

            TextField {
                id: searchField
                Layout.fillWidth: true
                enabled: root.pageController.currentFile.length > 0
                Keys.onReturnPressed: event => {
                    root.findInFile(Boolean(event.modifiers & Qt.ShiftModifier))
                    event.accepted = true
                }
            }

            Button {
                text: qsTranslate("OtherLogsPage", "Find")
                enabled: root.pageController.currentFile.length > 0
                onClicked: root.findInFile(false)
            }
        }
    }

    Shortcut {
        sequence: StandardKey.Find
        onActivated: {
            searchField.forceActiveFocus()
            searchField.selectAll()
        }
    }

    Shortcut {
        sequence: StandardKey.FindNext
        onActivated: root.findInFile(false)
    }

    Shortcut {
        sequence: StandardKey.FindPrevious
        onActivated: root.findInFile(true)
    }
}
