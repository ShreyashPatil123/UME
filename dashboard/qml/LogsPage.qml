import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Text {
            text: "Engine Logs Console"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#050608"
            border.color: Theme.cardBorder
            radius: Theme.radiusLarge

            ScrollView {
                anchors.fill: parent
                anchors.margins: Theme.paddingMedium
                clip: true

                TextArea {
                    textFormat: TextEdit.RichText
                    readOnly: true
                    font.family: "Consolas"
                    font.pixelSize: 12
                    color: Theme.textPrimary
                    text: "<font color='#50FA7B'>[INFO 12:04:01]</font> UME v3 core initialized successfully.<br/>" +
                          "<font color='#50FA7B'>[INFO 12:04:02]</font> Journal segment 001 opened for writing.<br/>" +
                          "<font color='#FFB86C'>[WARN 12:04:05]</font> Stride pattern matching confidence low (0.34) for Object ID 5.<br/>" +
                          "<font color='#FF5555'>[CRIT 12:04:32]</font> High VRAM pressure eviction routine triggered.<br/>" +
                          "<font color='#50FA7B'>[INFO 12:04:35]</font> Successfully migrated Object ID 42 to RAM."
                }
            }
        }
    }
}
