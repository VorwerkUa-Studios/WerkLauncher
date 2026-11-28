pragma Singleton
import QtQuick

QtObject {
    id: root

    property bool isBright: false
    property color accent: "#ffae00"

    // Background surfaces (Dark Mode = deep Obsidian/Zinc, Light Mode = clean Apple Gray)
    readonly property color bgRoot: isBright ? "#f5f5f7" : "#0f1115"
    readonly property color bgSidebar: isBright ? "#eaebee" : "#14171d"
    readonly property color bgHeader: isBright ? "#ffffff" : "#181b22"
    readonly property color bgCard: isBright ? "#ffffff" : "#1c2029"
    readonly property color bgCardHover: isBright ? "#f9fafb" : "#242935"
    readonly property color bgCardSelected: Qt.tint(bgCard, Qt.rgba(accent.r, accent.g, accent.b, 0.16))
    readonly property color bgInput: isBright ? "#ffffff" : "#13151b"
    readonly property color bgPill: isBright ? "#e4e5e9" : "#222631"
    readonly property color bgOverlay: isBright ? "rgba(0, 0, 0, 0.4)" : "rgba(0, 0, 0, 0.65)"
    readonly property color bgGlass: isBright ? "rgba(255, 255, 255, 0.75)" : "rgba(20, 23, 29, 0.75)"

    // Borders & Dividers
    readonly property color borderSubtle: isBright ? "rgba(0, 0, 0, 0.06)" : "rgba(255, 255, 255, 0.06)"
    readonly property color borderNormal: isBright ? "rgba(0, 0, 0, 0.12)" : "rgba(255, 255, 255, 0.10)"
    readonly property color borderHover: isBright ? "rgba(0, 0, 0, 0.22)" : "rgba(255, 255, 255, 0.22)"
    readonly property color borderActive: accent

    // Texts & Icons
    readonly property color textPrimary: isBright ? "#111827" : "#f4f4f6"
    readonly property color textSecondary: isBright ? "#4b5563" : "#9ca3af"
    readonly property color textMuted: isBright ? "#9ca3af" : "#6b7280"
    readonly property color textOnAccent: contrastForeground(accent)

    // Semantic status colors
    readonly property color statusRunning: "#10b981"
    readonly property color statusError: "#ef4444"
    readonly property color statusWarning: "#f59e0b"
    readonly property color statusInfo: "#3b82f6"

    // Typography
    readonly property string fontFamily: "Segoe UI, -apple-system, BlinkMacSystemFont, Roboto, sans-serif"
    readonly property string fontMono: "Cascadia Code, Consolas, Courier New, monospace"

    readonly property int fontSizeCaption: 11
    readonly property int fontSizeSmall: 12
    readonly property int fontSizeBody: 13
    readonly property int fontSizeSubheading: 15
    readonly property int fontSizeHeading: 18
    readonly property int fontSizeTitle: 22

    // Radii
    readonly property int radiusXs: 4
    readonly property int radiusSm: 6
    readonly property int radiusMd: 10
    readonly property int radiusLg: 14
    readonly property int radiusXl: 18
    readonly property int radiusPill: 999

    // Animation durations
    readonly property int animFast: 120
    readonly property int animNormal: 200
    readonly property int animSlow: 350

    function contrastForeground(c) {
        var col = Qt.color(c)
        var l = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b
        return l > 0.55 ? "#000000" : "#ffffff"
    }

    function alpha(colorVal, a) {
        var col = Qt.color(colorVal)
        return Qt.rgba(col.r, col.g, col.b, a)
    }
}
