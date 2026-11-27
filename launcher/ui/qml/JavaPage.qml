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

    ScrollView {
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: Math.max(0, Math.min(860, parent.width - 40))
            x: (parent.width - width) / 2
            spacing: 16

            Item { Layout.fillWidth: true; Layout.preferredHeight: 8 }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: memoryContent.implicitHeight + 32
                radius: 10
                color: palette.base
                border.color: palette.mid

                ColumnLayout {
                    id: memoryContent
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    Text {
                        text: qsTranslate("JavaPage", "Memory")
                        color: palette.windowText
                        font.pixelSize: 17
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { Layout.fillWidth: true; text: qsTranslate("JavaPage", "Minimum memory allocation"); color: palette.windowText }
                        SpinBox {
                            from: 128
                            to: root.pageController.memoryMaximum
                            stepSize: 128
                            editable: true
                            value: root.pageController.minMemory
                            onValueModified: root.pageController.minMemory = value
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { Layout.fillWidth: true; text: qsTranslate("JavaPage", "Maximum memory allocation"); color: palette.windowText }
                        SpinBox {
                            from: 128
                            to: root.pageController.memoryMaximum
                            stepSize: 128
                            editable: true
                            value: root.pageController.maxMemory
                            onValueModified: root.pageController.maxMemory = value
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { Layout.fillWidth: true; text: qsTranslate("JavaPage", "PermGen"); color: palette.windowText }
                        SpinBox {
                            from: 64
                            to: root.pageController.memoryMaximum
                            stepSize: 8
                            editable: true
                            value: root.pageController.permGen
                            onValueModified: root.pageController.permGen = value
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: runtimeContent.implicitHeight + 32
                radius: 10
                color: palette.base
                border.color: palette.mid

                ColumnLayout {
                    id: runtimeContent
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    Text {
                        text: qsTranslate("JavaPage", "Java Runtime")
                        color: palette.windowText
                        font.pixelSize: 17
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: qsTranslate("JavaPage", "Java path"); color: palette.windowText }
                        TextField {
                            Layout.fillWidth: true
                            text: root.pageController.javaPath
                            onTextEdited: root.pageController.javaPath = text
                        }
                        Button {
                            text: qsTranslate("JavaPage", "...")
                            onClicked: root.pageController.browseJava()
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: qsTranslate("JavaPage", "JVM arguments"); color: palette.windowText }
                        TextField {
                            Layout.fillWidth: true
                            text: root.pageController.jvmArgs
                            onTextEdited: root.pageController.jvmArgs = text
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Item { Layout.fillWidth: true }
                        Button {
                            text: qsTranslate("JavaPage", "Auto-detect...")
                            onClicked: root.pageController.detectJava()
                        }
                        Button {
                            text: qsTranslate("JavaPage", "Test")
                            onClicked: root.pageController.testJava()
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true; Layout.preferredHeight: 8 }
        }
    }
}
