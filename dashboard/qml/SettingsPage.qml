import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Application Settings"
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
                    Text { text: "UI Theme Mode:"; color: Theme.textPrimary; font.bold: true; Layout.preferredWidth: 200 }
                    ComboBox {
                        model: ["Dracula Dark (Industrial)", "Classic Light", "High Contrast"]
                        currentIndex: 0
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Telemetry Refresh Rate:"; color: Theme.textPrimary; font.bold: true; Layout.preferredWidth: 200 }
                    ComboBox {
                        model: ["500 ms", "1000 ms", "2000 ms", "5000 ms"]
                        currentIndex: 1
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Telemetry Server API Connection:"; color: Theme.textPrimary; font.bold: true; Layout.preferredWidth: 200 }
                    TextField {
                        text: "ws://localhost:8080/telemetry"
                        placeholderText: "ws://ip:port/telemetry"
                        Layout.preferredWidth: 300
                    }
                }

                Item { Layout.fillHeight: true } // spacer
            }
        }
    }
}
