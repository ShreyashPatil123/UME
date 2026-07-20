import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Real-Time Engine Timeline"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        ListView {
            id: timeList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            clip: true
            model: [
                { "time": "12:04:32.102", "event": "OBJECT_CREATED", "desc": "Allocated 4096 bytes in RAM tier" },
                { "time": "12:04:35.452", "event": "STRIDE_DETECTED", "desc": "Stride pattern confidence warmed to 0.85" },
                { "time": "12:04:35.801", "event": "PREDICTED_TIER", "desc": "Object ID 42 expected tier: VRAM" },
                { "time": "12:04:36.002", "event": "MIGRATION_DISPATCHED", "desc": "Evicting object ID 42 to VRAM" }
            ]

            delegate: Rectangle {
                width: timeList.width
                height: 50
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusMedium

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingSmall
                    spacing: Theme.paddingMedium

                    Text {
                        text: modelData.time
                        color: Theme.primary
                        font.pixelSize: 12
                    }

                    Rectangle {
                        width: 120
                        height: 20
                        color: "#242838"
                        radius: Theme.radiusSmall
                        Text {
                            anchors.centerIn: parent
                            text: modelData.event
                            color: Theme.textPrimary
                            font.bold: true
                            font.pixelSize: 10
                        }
                    }

                    Text {
                        text: modelData.desc
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
