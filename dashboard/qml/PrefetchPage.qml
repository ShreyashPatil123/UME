import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Intelligent Prefetch Engine"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.paddingMedium

            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                ColumnLayout {
                    anchors.centerIn: parent
                    Text { text: "Prefetch Hit Rate"; color: Theme.textSecondary; font.pixelSize: 12 }
                    Text { text: "91.2%"; color: Theme.success; font.pixelSize: 28; font.bold: true }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                ColumnLayout {
                    anchors.centerIn: parent
                    Text { text: "Saved Bandwidth Overhead"; color: Theme.textSecondary; font.pixelSize: 12 }
                    Text { text: "420.5 GB"; color: Theme.primary; font.pixelSize: 28; font.bold: true }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                ColumnLayout {
                    anchors.centerIn: parent
                    Text { text: "Wasted/Cancelled Prefetches"; color: Theme.textSecondary; font.pixelSize: 12 }
                    Text { text: "1.4%"; color: Theme.danger; font.pixelSize: 28; font.bold: true }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.cardBackground
            border.color: Theme.cardBorder
            radius: Theme.radiusLarge

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.paddingMedium
                Text {
                    text: "Prefetch Channels & Queue Status"
                    color: Theme.textPrimary
                    font.pixelSize: 16
                    font.bold: true
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Text {
                        anchors.centerIn: parent
                        text: "All Prefetch Channels Idle (Queue depth: 0)"
                        color: Theme.textSecondary
                    }
                }
            }
        }
    }
}
