#pragma once

/// @file mock_backend.h
/// @brief Mock telemetry data generation interface for the Qt UI.

#include <QObject>
#include <QTimer>
#include <string>
#include <vector>

namespace ume::dashboard {

struct AppMetric {
    QString name;
    double vram_mb{0.0};
    double ram_mb{0.0};
    double ssd_mb{0.0};
    double bandwidth_gbps{0.0};
    int migration_count{0};
    double score{100.0};
};

struct UIRecommendation {
    QString category;
    QString severity;
    QString description;
    QString root_cause;
    QString suggested_action;
    int estimated_benefit_us{0};
    double confidence{0.9};
    double health_score{100.0};
};

struct UISimulationPlan {
    int plan_id{0};
    QString name;
    QString target_tier;
    int latency_us{0};
    double bandwidth_gbps{0.0};
    double hit_ratio{1.0};
    double score{100.0};
};

class MockBackend : public QObject {
    Q_OBJECT
public:
    explicit MockBackend(QObject* parent = nullptr);
    ~MockBackend() override = default;

    // Getter methods for viewmodels
    double vram_usage_mb() const { return vram_usage_mb_; }
    double ram_usage_mb() const { return ram_usage_mb_; }
    double ssd_usage_mb() const { return ssd_usage_mb_; }
    double migration_rate() const { return migration_rate_; }
    double system_health() const { return system_health_; }
    double prediction_accuracy() const { return prediction_accuracy_; }
    double simulation_accuracy() const { return simulation_accuracy_; }

    std::vector<AppMetric> get_applications() const;
    std::vector<UIRecommendation> get_recommendations() const;
    std::vector<UISimulationPlan> get_simulation_plans() const;

signals:
    void telemetry_updated();

private slots:
    void tick();

private:
    QTimer timer_;
    double vram_usage_mb_{12500.0};
    double ram_usage_mb_{24100.0};
    double ssd_usage_mb_{45000.0};
    double migration_rate_{1.2};
    double system_health_{98.5};
    double prediction_accuracy_{0.92};
    double simulation_accuracy_{0.96};
};

} // namespace ume::dashboard
