import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Engine Policies Configuration"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
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
                spacing: Theme.paddingLarge

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Scheduler Policy:"; color: Theme.textPrimary; font.bold: true; Layout.preferredWidth: 200 }
                    ComboBox {
                        model: ["Greedy Tiering", "Cost-Benefit Hysteresis", "LRU Spill Cache"]
                        currentIndex: 1
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Prefetch Policy:"; color: Theme.textPrimary; font.bold: true; Layout.preferredWidth: 200 }
                    ComboBox {
                        model: ["Strict Linear", "Stride Learner", "No-Evict Conservative"]
                        currentIndex: 1
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Optimization Policy:"; color: Theme.textPrimary; font.bold: true; Layout.preferredWidth: 200 }
                    ComboBox {
                        model: ["Latency Minimization", "Bandwidth Constrained", "Power Saving"]
                        currentIndex: 0
                    }
                }

                Item { Layout.fillHeight: true } // spacer
            }
        }
    }
}
