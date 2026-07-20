import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Physical Memory Tiers Map"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        // Live Allocation Map view
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.cardBackground
            border.color: Theme.cardBorder
            radius: Theme.radiusLarge

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.paddingMedium
                spacing: Theme.paddingMedium

                Text {
                    text: "Unified Virtual Allocations Heatmap"
                    color: Theme.textPrimary
                    font.pixelSize: 16
                    font.bold: true
                }

                // Layout simulation representation
                GridView {
                    id: grid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    cellWidth: 40
                    cellHeight: 40
                    clip: true
                    model: 128 // 128 virtual memory blocks

                    delegate: Rectangle {
                        width: 32
                        height: 32
                        radius: Theme.radiusSmall
                        // Alternating tier colors
                        color: index % 7 === 0 ? Theme.warning :
                               index % 5 === 0 ? Theme.secondary :
                               index % 3 === 0 ? Theme.primary : "#1B1D2E"
                        border.color: Theme.cardBorder

                        ToolTip.visible: mouseArea.containsMouse
                        ToolTip.text: "Block ID: " + index + "\nStatus: Allocated"

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }
                }
            }
        }
    }
}
