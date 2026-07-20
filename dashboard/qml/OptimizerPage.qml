import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Memory Optimizer Telemetry"
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
                spacing: Theme.paddingMedium

                Text {
                    text: "Asynchronous Migration Queue"
                    color: Theme.textPrimary
                    font.pixelSize: 16
                    font.bold: true
                }

                ListView {
                    id: optList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8
                    clip: true
                    model: [
                        { "id": 1042, "source": "RAM", "dest": "VRAM", "size": "64 MB", "status": "In Progress" },
                        { "id": 1089, "source": "VRAM", "dest": "RAM", "size": "128 MB", "status": "Queued" },
                        { "id": 1102, "source": "RAM", "dest": "SSD", "size": "256 MB", "status": "Completed" }
                    ]

                    delegate: Rectangle {
                        width: optList.width
                        height: 60
                        color: "#1F2330"
                        radius: Theme.radiusMedium

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: Theme.paddingMedium
                            spacing: Theme.paddingLarge

                            Text {
                                text: "MIGRATION #" + modelData.id
                                color: Theme.textPrimary
                                font.bold: true
                            }
                            Text {
                                text: modelData.source + " ➔ " + modelData.dest
                                color: Theme.primary
                            }
                            Text {
                                text: "Size: " + modelData.size
                                color: Theme.textSecondary
                            }
                            Text {
                                text: modelData.status
                                color: modelData.status === "Completed" ? Theme.success : Theme.warning
                                Layout.alignment: Qt.AlignRight
                            }
                        }
                    }
                }
            }
        }
    }
}
