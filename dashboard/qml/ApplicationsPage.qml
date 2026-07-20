import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Registered Active Applications"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        // Applications List View
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.paddingSmall
            clip: true
            model: memoryVM.applications

            delegate: Rectangle {
                width: listView.width
                height: 80
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium
                    spacing: Theme.paddingLarge

                    ColumnLayout {
                        Layout.preferredWidth: 200
                        Text {
                            text: modelData.name
                            color: Theme.textPrimary
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Text {
                            text: "Allocations Profile"
                            color: Theme.textSecondary
                            font.pixelSize: 12
                        }
                    }

                    // Tiers
                    RowLayout {
                        spacing: Theme.paddingMedium
                        Layout.fillWidth: true

                        ColumnLayout {
                            Text { text: "VRAM"; color: Theme.textSecondary; font.pixelSize: 11 }
                            Text { text: modelData.vram_mb.toFixed(0) + " MB"; color: Theme.primary; font.bold: true }
                        }
                        ColumnLayout {
                            Text { text: "RAM"; color: Theme.textSecondary; font.pixelSize: 11 }
                            Text { text: modelData.ram_mb.toFixed(0) + " MB"; color: Theme.secondary; font.bold: true }
                        }
                        ColumnLayout {
                            Text { text: "SSD"; color: Theme.textSecondary; font.pixelSize: 11 }
                            Text { text: modelData.ssd_mb.toFixed(0) + " MB"; color: Theme.warning; font.bold: true }
                        }
                    }

                    // Score
                    ColumnLayout {
                        Layout.alignment: Qt.AlignRight
                        Text { text: "Placement Efficiency"; color: Theme.textSecondary; font.pixelSize: 11 }
                        Text { text: modelData.score.toFixed(1) + "%"; color: Theme.success; font.pixelSize: 18; font.bold: true }
                    }
                }
            }
        }
    }
}
