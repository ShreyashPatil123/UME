import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Pattern Learning Database"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        ListView {
            id: patList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            clip: true
            model: [
                { "obj": "Tensor Weight #2", "stride": "4096 bytes", "confidence": "0.85", "type": "Sequential Stride" },
                { "obj": "Lookup Embedding #15", "stride": "16384 bytes", "confidence": "0.92", "type": "Sequential Stride" },
                { "obj": "Matrix Multiply Cache", "stride": "Random", "confidence": "0.34", "type": "Random Access" }
            ]

            delegate: Rectangle {
                width: patList.width
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
                        Text { text: "Pattern Model: " + modelData.type; color: Theme.textSecondary; font.pixelSize: 11 }
                    }

                    Text {
                        text: "Stride: " + modelData.stride
                        color: Theme.primary
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignRight
                        Text { text: "Learning Confidence"; color: Theme.textSecondary; font.pixelSize: 11 }
                        Text { text: (modelData.confidence * 100).toFixed(0) + "%"; color: Theme.success; font.bold: true }
                    }
                }
            }
        }
    }
}
