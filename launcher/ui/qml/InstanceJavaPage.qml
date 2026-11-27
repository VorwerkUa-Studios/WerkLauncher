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

                    CheckBox {
                        text: qsTranslate("InstanceJavaPage", "Override memory settings")
                        checked: root.pageController.overrideMemory
                        onToggled: root.pageController.overrideMemory = checked
                    }

                    ColumnLayout {
                        Layout.leftMargin: 28
                        Layout.fillWidth: true
                        enabled: root.pageController.overrideMemory
                        opacity: enabled ? 1 : 0.55
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Text { Layout.fillWidth: true; text: qsTranslate("InstanceJavaPage", "Minimum memory allocation"); color: palette.windowText }
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
                            Text { Layout.fillWidth: true; text: qsTranslate("InstanceJavaPage", "Maximum memory allocation"); color: palette.windowText }
                            SpinBox {
                                from: 128
                                to: root.pageController.memoryMaximum
                                stepSize: 128
                                editable: true
                                value: root.pageController.maxMemory
                                onValueModified: root.pageController.maxMemory = value
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: javaContent.implicitHeight + 32
                radius: 10
                color: palette.base
                border.color: palette.mid

                ColumnLayout {
                    id: javaContent
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    CheckBox {
                        text: qsTranslate("InstanceJavaPage", "Override Java installation")
                        checked: root.pageController.overrideJavaLocation
                        onToggled: root.pageController.overrideJavaLocation = checked
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 28
                        enabled: root.pageController.overrideJavaLocation
                        opacity: enabled ? 1 : 0.55
                        Text { text: qsTranslate("InstanceJavaPage", "Java path"); color: palette.windowText }
                        TextField {
                            Layout.fillWidth: true
                            text: root.pageController.javaPath
                            onTextEdited: root.pageController.javaPath = text
                        }
                        Button {
                            text: qsTranslate("InstanceJavaPage", "...")
                            onClicked: root.pageController.browseJava()
                        }
                    }

                    CheckBox {
                        text: qsTranslate("InstanceJavaPage", "Override JVM arguments")
                        checked: root.pageController.overrideJvmArgs
                        onToggled: root.pageController.overrideJvmArgs = checked
                    }

                    TextField {
                        Layout.fillWidth: true
                        Layout.leftMargin: 28
                        enabled: root.pageController.overrideJvmArgs
                        opacity: enabled ? 1 : 0.55
                        text: root.pageController.jvmArgs
                        onTextEdited: root.pageController.jvmArgs = text
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Item { Layout.fillWidth: true }
                        Button {
                            text: qsTranslate("InstanceJavaPage", "Auto-detect...")
                            onClicked: root.pageController.detectJava()
                        }
                        Button {
                            text: qsTranslate("InstanceJavaPage", "Test")
                            onClicked: root.pageController.testJava()
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true; Layout.preferredHeight: 8 }
        }
    }
}
