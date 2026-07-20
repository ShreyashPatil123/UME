import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLarge
        spacing: Theme.paddingMedium

        // Header
        Text {
            text: "UME Observability Dashboard"
            font.pixelSize: 24
            font.bold: true
            color: Theme.textPrimary
        }

        // Summary row cards
        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.paddingMedium

            // Health Card
            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    Text {
                        text: "System Health Score"
                        color: Theme.textSecondary
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: dashboardVM.systemHealth.toFixed(1) + "%"
                        color: Theme.success
                        font.pixelSize: 32
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            // VRAM Card
            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    Text {
                        text: "Active VRAM Usage"
                        color: Theme.textSecondary
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: (dashboardVM.vramUsageRead / 1024).toFixed(2) + " GB"
                        color: Theme.primary
                        font.pixelSize: 28
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            // RAM Card
            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    Text {
                        text: "Active System RAM"
                        color: Theme.textSecondary
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: (dashboardVM.ramUsageRead / 1024).toFixed(2) + " GB"
                        color: Theme.secondary
                        font.pixelSize: 28
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            // SSD Card
            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    Text {
                        text: "Active SSD Spill"
                        color: Theme.textSecondary
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: (dashboardVM.ssdUsageRead / 1024).toFixed(2) + " GB"
                        color: Theme.warning
                        font.pixelSize: 28
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }

        // Live telemetry rates
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.paddingMedium

            // Latency & Migration Chart placeholder
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium

                    Text {
                        text: "Migration Rate & Bandwidth Timeline"
                        color: Theme.textPrimary
                        font.pixelSize: 16
                        font.bold: true
                    }

                    // Visual simulated line chart
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Rectangle {
                            anchors.fill: parent
                            color: "#1B1D2E"
                            radius: Theme.radiusMedium

                            Text {
                                anchors.centerIn: parent
                                text: "Live Timeline Chart (Active Rates: " + dashboardVM.migrationRate.toFixed(2) + " GB/s)"
                                color: Theme.textSecondary
                                font.pixelSize: 14
                            }
                        }
                    }
                }
            }

            // Accuracy Metrics card
            Rectangle {
                width: 320
                Layout.fillHeight: true
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLarge

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium
                    spacing: Theme.paddingMedium

                    Text {
                        text: "Engine Diagnostics"
                        color: Theme.textPrimary
                        font.pixelSize: 16
                        font.bold: true
                    }

                    // Prediction Accuracy Indicator
                    ColumnLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Prediction Accuracy"
                            color: Theme.textSecondary
                            font.pixelSize: 12
                        }
                        ProgressBar {
                            Layout.fillWidth: true
                            value: dashboardVM.predictionAccuracy
                            background: Rectangle {
                                implicitHeight: 6
                                color: "#2E3440"
                                radius: 3
                            }
                            contentItem: Item {
                                implicitHeight: 6
                                Rectangle {
                                    width: parent.width * parent.parent.value
                                    height: parent.height
                                    color: Theme.primary
                                    radius: 3
                                }
                            }
                        }
                        Text {
                            text: (dashboardVM.predictionAccuracy * 100).toFixed(1) + "%"
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                        }
                    }

                    // Simulation Accuracy Indicator
                    ColumnLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Digital Twin Sim Accuracy"
                            color: Theme.textSecondary
                            font.pixelSize: 12
                        }
                        ProgressBar {
                            Layout.fillWidth: true
                            value: dashboardVM.simulationAccuracy
                            background: Rectangle {
                                implicitHeight: 6
                                color: "#2E3440"
                                radius: 3
                            }
                            contentItem: Item {
                                implicitHeight: 6
                                Rectangle {
                                    width: parent.width * parent.parent.value
                                    height: parent.height
                                    color: Theme.secondary
                                    radius: 3
                                }
                            }
                        }
                        Text {
                            text: (dashboardVM.simulationAccuracy * 100).toFixed(1) + "%"
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                        }
                    }
                }
            }
        }
    }
}
