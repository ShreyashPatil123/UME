import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 800
    title: "Unified Memory Engine (UME) Dashboard"
    color: Theme.background

    // Navigation state
    property string currentPage: "Dashboard"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left Navigation Rail
        Rectangle {
            Layout.fillHeight: true
            width: 240
            color: Theme.cardBackground

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.paddingMedium
                spacing: Theme.paddingSmall

                // Logo/Header
                Text {
                    text: "UME V3 PLATFORM"
                    font.pixelSize: 18
                    font.bold: true
                    color: Theme.primary
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: Theme.paddingLarge
                }

                // Nav list
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    Column {
                        width: parent.width
                        spacing: 4

                        // Navigation Button Helper component within Main
                        Repeater {
                            model: [
                                "Dashboard", "Memory", "Applications", "Optimizer",
                                "Prefetch", "Pattern Learning", "Prediction",
                                "Digital Twin", "Memory Advisor", "Timeline",
                                "Statistics", "Policies", "Logs", "Settings", "About"
                            ]

                            delegate: Button {
                                width: 200
                                text: modelData
                                flat: true
                                contentItem: Text {
                                    text: parent.text
                                    color: (window.currentPage === parent.text) ? Theme.primary : Theme.textSecondary
                                    font.pixelSize: 14
                                    font.bold: window.currentPage === parent.text
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 16
                                }

                                background: Rectangle {
                                    color: (window.currentPage === parent.text) ? "#202438" : "transparent"
                                    radius: Theme.radiusMedium
                                }

                                onClicked: {
                                    window.currentPage = parent.text;
                                }
                            }
                        }
                    }
                }

                // Bottom footer status
                Row {
                    spacing: 8
                    Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: Theme.success
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: "Engine Connected (Mock)"
                        color: Theme.textSecondary
                        font.pixelSize: 12
                    }
                }
            }
        }

        // Divider
        Rectangle {
            width: 1
            Layout.fillHeight: true
            color: Theme.cardBorder
        }

        // Main Page content area
        StackLayout {
            id: contentStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: getPageIndex(window.currentPage)

            DashboardPage {}
            MemoryPage {}
            ApplicationsPage {}
            OptimizerPage {}
            PrefetchPage {}
            PatternLearningPage {}
            PredictionPage {}
            DigitalTwinPage {}
            MemoryAdvisorPage {}
            TimelinePage {}
            StatisticsPage {}
            PoliciesPage {}
            LogsPage {}
            SettingsPage {}
            AboutPage {}
        }
    }

    function getPageIndex(pageName) {
        var pages = [
            "Dashboard", "Memory", "Applications", "Optimizer",
            "Prefetch", "Pattern Learning", "Prediction",
            "Digital Twin", "Memory Advisor", "Timeline",
            "Statistics", "Policies", "Logs", "Settings", "About"
        ];
        var idx = pages.indexOf(pageName);
        return idx >= 0 ? idx : 0;
    }
}
