import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Historical Statistics Charts"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.paddingMedium

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium
                    Text { text: "Average Migration Latency"; color: Theme.textPrimary; font.bold: true }
                    Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; color: "#1B1D2E"; radius: Theme.radiusMedium; Text { anchors.centerIn: parent; text: "Simulated Latency Area Chart"; color: Theme.textSecondary } }
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
                    Text { text: "Active Channel Bandwidth"; color: Theme.textPrimary; font.bold: true }
                    Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; color: "#1B1D2E"; radius: Theme.radiusMedium; Text { anchors.centerIn: parent; text: "Simulated Bandwidth Bar Chart"; color: Theme.textSecondary } }
                }
            }
        }
    }
}
