pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var pageController
    property bool followTail: true

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    function findInLog(reverse) {
        const row = pageController.findNext(searchField.text, reverse)
        if (row >= 0)
            logList.positionViewAtIndex(row, ListView.Contain)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            CheckBox {
                text: qsTranslate("LogPage", "Keep updating")
                checked: root.pageController.tracking
                onToggled: root.pageController.tracking = checked
            }

            CheckBox {
                text: qsTranslate("LogPage", "Wrap lines")
                checked: root.pageController.wrapLines
                onToggled: root.pageController.wrapLines = checked
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTranslate("LogPage", "Copy")
                enabled: root.pageController.hasLogModel
                onClicked: root.pageController.copyLog()
            }

            Button {
                text: qsTranslate("LogPage", "Upload")
                enabled: root.pageController.hasLogModel
                onClicked: root.pageController.uploadLog()
            }

            Button {
                text: qsTranslate("LogPage", "Clear")
                enabled: root.pageController.hasLogModel
                onClicked: root.pageController.clearLog()
            }
        }

        ListView {
            id: logList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 0
            model: root.pageController.logModel
            contentWidth: root.pageController.wrapLines
                          ? width : Math.max(width, contentItem.childrenRect.width)

            delegate: Rectangle {
                id: logDelegate
                required property int index
                required property string display
                required property color foreground
                required property color background
                required property font font

                width: root.pageController.wrapLines
                       ? logList.width : Math.max(logList.width, messageText.implicitWidth)
                height: messageText.implicitHeight
                color: logDelegate.index === root.pageController.foundRow
                       ? palette.highlight
                       : (background.a > 0 ? background : palette.base)

                TextEdit {
                    id: messageText
                    x: 4
                    width: parent.width - 8
                    text: parent.display
                    color: logDelegate.index === root.pageController.foundRow
                           ? palette.highlightedText : parent.foreground
                    font: parent.font
                    wrapMode: root.pageController.wrapLines
                              ? TextEdit.WrapAnywhere : TextEdit.NoWrap
                    textFormat: TextEdit.PlainText
                    readOnly: true
                    selectByMouse: true
                }
            }

            ScrollBar.vertical: ScrollBar {}
            ScrollBar.horizontal: ScrollBar {
                policy: root.pageController.wrapLines
                        ? ScrollBar.AlwaysOff : ScrollBar.AsNeeded
            }

            Label {
                anchors.centerIn: parent
                visible: logList.count === 0
                text: root.pageController.hasLogModel
                      ? qsTranslate("LogPage", "No log messages yet.")
                      : qsTranslate("LogPage", "The instance is not running.")
                color: palette.windowText
                opacity: 0.6
            }

            Connections {
                target: root.pageController.logModel

                function onRowsAboutToBeInserted() {
                    root.followTail = logList.atYEnd
                                    || logList.contentY
                                       >= logList.contentHeight - logList.height - 1
                }

                function onRowsInserted() {
                    if (root.followTail)
                        Qt.callLater(() => logList.positionViewAtEnd())
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: qsTranslate("LogPage", "Search:")
            }

            TextField {
                id: searchField
                Layout.fillWidth: true
                Keys.onReturnPressed: event => {
                    root.findInLog(Boolean(event.modifiers & Qt.ShiftModifier))
                    event.accepted = true
                }
            }

            Button {
                text: qsTranslate("LogPage", "Find")
                onClicked: root.findInLog(false)
            }

            Button {
                text: qsTranslate("LogPage", "Bottom")
                onClicked: logList.positionViewAtEnd()
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
        onActivated: root.findInLog(false)
    }

    Shortcut {
        sequence: StandardKey.FindPrevious
        onActivated: root.findInLog(true)
    }
}
