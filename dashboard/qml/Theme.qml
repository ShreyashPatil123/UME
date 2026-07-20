pragma Singleton
import QtQuick

QtObject {
    // Colors
    readonly property color background: "#0D0E12"
    readonly property color cardBackground: "#161821"
    readonly property color cardBorder: "#242838"
    readonly property color primary: "#00E5FF"
    readonly property color secondary: "#BD93F9"
    readonly property color success: "#50FA7B"
    readonly property color warning: "#FFB86C"
    readonly property color danger: "#FF5555"
    readonly property color textPrimary: "#FFFFFF"
    readonly property color textSecondary: "#8A94A6"

    // Spacing
    readonly property int paddingLarge: 24
    readonly property int paddingMedium: 16
    readonly property int paddingSmall: 8

    // Radius
    readonly property int radiusLarge: 12
    readonly property int radiusMedium: 8
    readonly property int radiusSmall: 4
}
