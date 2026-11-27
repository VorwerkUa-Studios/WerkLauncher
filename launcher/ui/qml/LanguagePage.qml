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
        id: scrollView
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            id: mainContent
            x: (scrollView.availableWidth - width) / 2
            width: Math.max(0, Math.min(860, scrollView.availableWidth - 40))
            spacing: 20

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: qsTranslate("LanguagePage", "Language")
                    color: palette.windowText
                    font.pixelSize: 22
                    font.bold: true
                }

                Text {
                    text: qsTranslate("LanguagePage", "Select application interface language")
                    color: palette.windowText
                    opacity: 0.7
                    font.pixelSize: 13
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                LanguageCard {
                    Layout.fillWidth: true
                    flagText: "🇺🇦"
                    langCode: "UK"
                    langName: "Українська"
                    langNative: "Українська мова"
                    selected: root.pageController.selectedLanguage === "uk"
                    onClicked: root.pageController.selectedLanguage = "uk"
                }

                LanguageCard {
                    Layout.fillWidth: true
                    flagText: "🇬🇧"
                    langCode: "EN"
                    langName: "English"
                    langNative: "English (US)"
                    selected: root.pageController.selectedLanguage === "en_US" || root.pageController.selectedLanguage === ""
                    onClicked: root.pageController.selectedLanguage = "en_US"
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
            }
        }
    }

    component LanguageCard: Rectangle {
        id: card
        required property string flagText
        required property string langCode
        required property string langName
        required property string langNative
        required property bool selected
        signal clicked()

        implicitHeight: 76
        radius: 12
        color: selected ? Qt.rgba(palette.highlight.r, palette.highlight.g, palette.highlight.b, 0.15)
                        : (mouseArea.containsMouse ? Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.25)
                                                  : palette.base)
        border.color: selected ? palette.highlight : (mouseArea.containsMouse ? palette.mid : Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.5))
        border.width: selected ? 2 : 1

        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on border.color { ColorAnimation { duration: 150 } }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: card.clicked()
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            Text {
                text: card.flagText
                font.pixelSize: 28
                verticalAlignment: Text.AlignVCenter
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                RowLayout {
                    spacing: 8
                    Text {
                        text: card.langName
                        color: palette.windowText
                        font.pixelSize: 16
                        font.bold: true
                    }
                    Rectangle {
                        implicitWidth: codeText.implicitWidth + 8
                        implicitHeight: codeText.implicitHeight + 4
                        radius: 4
                        color: Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.3)
                        Text {
                            id: codeText
                            anchors.centerIn: parent
                            text: card.langCode
                            color: palette.windowText
                            font.pixelSize: 11
                            font.bold: true
                            opacity: 0.8
                        }
                    }
                }

                Text {
                    text: card.langNative
                    color: palette.windowText
                    opacity: 0.65
                    font.pixelSize: 12
                }
            }

            Rectangle {
                implicitWidth: 24
                implicitHeight: 24
                radius: 12
                color: card.selected ? palette.highlight : "transparent"
                border.color: card.selected ? palette.highlight : palette.mid
                border.width: card.selected ? 0 : 2

                Text {
                    anchors.centerIn: parent
                    text: "✓"
                    color: palette.highlightedText
                    font.bold: true
                    font.pixelSize: 14
                    visible: card.selected
                }
            }
        }
    }
}
