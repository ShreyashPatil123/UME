import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Digital Twin Strategy Simulator"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        ListView {
            id: twinList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            clip: true
            model: digitalTwinVM.simulationPlans

            delegate: Rectangle {
                width: twinList.width
                height: 80
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium
                    spacing: Theme.paddingLarge

                    ColumnLayout {
                        Layout.preferredWidth: 240
                        Text { text: modelData.name; color: Theme.textPrimary; font.bold: true }
                        Text { text: "Candidate Tier: " + modelData.target_tier; color: Theme.textSecondary; font.pixelSize: 11 }
                    }

                    ColumnLayout {
                        Text { text: "Latency"; color: Theme.textSecondary; font.pixelSize: 11 }
                        Text { text: modelData.latency_us + " us"; color: Theme.warning; font.bold: true }
                    }

                    ColumnLayout {
                        Text { text: "Bandwidth"; color: Theme.textSecondary; font.pixelSize: 11 }
                        Text { text: modelData.bandwidth_gbps.toFixed(1) + " GB/s"; color: Theme.primary; font.bold: true }
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignRight
                        Text { text: "Overall Score"; color: Theme.textSecondary; font.pixelSize: 11 }
                        Text { text: modelData.score.toFixed(1) + " pts"; color: Theme.success; font.pixelSize: 16; font.bold: true }
                    }
                }
            }
        }
    }
}
