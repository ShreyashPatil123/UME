import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Memory Advisor Decisions"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        ListView {
            id: advList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.paddingSmall
            clip: true
            model: advisorVM.recommendations

            delegate: Rectangle {
                width: advList.width
                height: 120
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium
                    spacing: Theme.paddingLarge

                    ColumnLayout {
                        Layout.preferredWidth: 80
                        Text {
                            text: modelData.severity
                            color: modelData.severity === "CRITICAL" ? Theme.danger : Theme.warning
                            font.bold: true
                            font.pixelSize: 14
                        }
                        Text {
                            text: modelData.category
                            color: Theme.textSecondary
                            font.pixelSize: 11
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Text { text: modelData.description; color: Theme.textPrimary; font.bold: true }
                        Text { text: "Root Cause: " + modelData.root_cause; color: Theme.textSecondary; font.pixelSize: 11 }
                        Text { text: "Suggested Action: " + modelData.suggested_action; color: Theme.primary; font.pixelSize: 11 }
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignRight
                        Text { text: "Est Benefit"; color: Theme.textSecondary; font.pixelSize: 11 }
                        Text { text: (modelData.estimated_benefit_us / 1000).toFixed(1) + " ms"; color: Theme.success; font.bold: true; font.pixelSize: 16 }
                    }
                }
            }
        }
    }
}
