#pragma once

/// @file dashboard_viewmodel.h
/// @brief ViewModel exposing engine statistics and state indicators to the UI.

#include "mock_backend.h"

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
    explicit DashboardViewModel(MockBackend* backend, QObject* parent = nullptr);
    ~DashboardViewModel() override = default;

    double vramUsageRead() const { return backend_->vram_usage_mb(); }
    double ramUsageRead() const { return backend_->ram_usage_mb(); }
    double ssdUsageRead() const { return backend_->ssd_usage_mb(); }
    double migrationRate() const { return backend_->migration_rate(); }
    double systemHealth() const { return backend_->system_health(); }
    double predictionAccuracy() const { return backend_->prediction_accuracy(); }
    double simulationAccuracy() const { return backend_->simulation_accuracy(); }

signals:
    void telemetryChanged();

private:
    MockBackend* backend_;
};

} // namespace ume::dashboard
