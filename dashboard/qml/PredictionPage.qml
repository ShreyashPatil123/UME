import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Hybrid Predictor Metrics"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        ListView {
            id: predList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            clip: true
            model: [
                { "obj": "Tensor Weight #2", "expected_tier": "VRAM", "confidence": "0.85" },
                { "obj": "Lookup Embedding #15", "expected_tier": "VRAM", "confidence": "0.92" },
                { "obj": "Cold Activation Output", "expected_tier": "SSD", "confidence": "0.78" }
            ]

            delegate: Rectangle {
                width: predList.width
                height: 70
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium
                    spacing: Theme.paddingLarge

                    ColumnLayout {
                        Layout.preferredWidth: 200
                        Text { text: modelData.obj; color: Theme.textPrimary; font.bold: true }
                        Text { text: "Active forecast model"; color: Theme.textSecondary; font.pixelSize: 11 }
                    }

                    Text {
                        text: "Expected: " + modelData.expected_tier
                        color: Theme.secondary
                        font.bold: true
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignRight
                        Text { text: "Forecast Confidence"; color: Theme.textSecondary; font.pixelSize: 11 }
                        Text { text: (modelData.confidence * 100).toFixed(0) + "%"; color: Theme.success; font.bold: true }
                    }
                }
            }
        }
    }
}
