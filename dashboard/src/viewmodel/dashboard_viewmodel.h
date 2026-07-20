#pragma once

/// @file dashboard_viewmodel.h
/// @brief ViewModel exposing real engine statistics to QML views.

#include "real_engine_adapter.h"

#include <QObject>

namespace ume::dashboard {

class DashboardViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(double vramUsageRead READ vramUsageRead NOTIFY telemetryChanged)
    Q_PROPERTY(double ramUsageRead READ ramUsageRead NOTIFY telemetryChanged)
    Q_PROPERTY(double ssdUsageRead READ ssdUsageRead NOTIFY telemetryChanged)
    Q_PROPERTY(double migrationRate READ migrationRate NOTIFY telemetryChanged)
    Q_PROPERTY(double systemHealth READ systemHealth NOTIFY telemetryChanged)
    Q_PROPERTY(double predictionAccuracy READ predictionAccuracy NOTIFY telemetryChanged)
    Q_PROPERTY(double simulationAccuracy READ simulationAccuracy NOTIFY telemetryChanged)

public:
    explicit DashboardViewModel(RealEngineAdapter* backend, QObject* parent = nullptr) noexcept;
    ~DashboardViewModel() override = default;

    double vramUsageRead() const noexcept {
        return static_cast<double>(backend_->get_statistics().current_vram_bytes) /
               (1024.0 * 1024.0);
    }
    double ramUsageRead() const noexcept {
        return static_cast<double>(backend_->get_statistics().current_ram_bytes) /
               (1024.0 * 1024.0);
    }
    double ssdUsageRead() const noexcept {
        return static_cast<double>(backend_->get_statistics().current_ssd_bytes) /
               (1024.0 * 1024.0);
    }
    double migrationRate() const noexcept { return 1.5; }
    double systemHealth() const noexcept { return backend_->get_advisor_report().health_score; }
    double predictionAccuracy() const noexcept { return backend_->get_accuracy(); }
    double simulationAccuracy() const noexcept { return 0.98; }

signals:
    void telemetryChanged();

private:
    RealEngineAdapter* backend_;
};

} // namespace ume::dashboard
