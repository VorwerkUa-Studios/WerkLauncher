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
        spacing: 16

        Text {
            Layout.fillWidth: true
            text: qsTranslate("LegacyUpgradePage", "Upgrade is required")
            color: palette.windowText
            font.pixelSize: 22
            font.bold: true
            wrapMode: Text.WordWrap
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Text {
                width: parent.width
                textFormat: Text.RichText
                text: qsTranslate("LegacyUpgradePage",
                                  "<p>MultiMC now supports old Minecraft versions and all the required features in the new (OneSix) instance format. As a consequence, the old (Legacy) format has been entirely disabled and old instances need to be upgraded.</p><p>The upgrade will create a new instance with the same contents as the current one, in the new format. The original instance will remain untouched, in case anything goes wrong in the process.</p><p>Please report any issues on our <a href=\"https://github.com/MultiMC/Launcher/issues\">github issues page</a>.</p><p>There is also a <a href=\"https://discord.gg/GtPmv93\">discord channel for testing here</a>.</p>")
                color: palette.windowText
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                onLinkActivated: link => Qt.openUrlExternally(link)
            }
        }

        Button {
            Layout.alignment: Qt.AlignRight
            text: qsTranslate("LegacyUpgradePage", "Upgrade the instance")
            enabled: root.pageController.upgradeEnabled
            onClicked: root.pageController.upgrade()
        }
    }
}
