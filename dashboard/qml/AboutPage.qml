import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "About UME Platform"
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
                anchors.centerIn: parent
                spacing: Theme.paddingMedium

                Text {
                    text: "Unified Memory Engine (UME) Dashboard"
                    color: Theme.primary
                    font.pixelSize: 20
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Version: 3.0.0 Stable Baseline"
                    color: Theme.textPrimary
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Universal AI Memory Virtualization Platform for Heterogeneous Tier Systems."
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Licensed under Apache-2.0 / MIT. Copyright (c) 2026 UME Contributors."
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
