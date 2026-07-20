/// @file mock_backend.cpp
/// @brief Telemetry mock updater implementation.

#include "mock_backend.h"

#include <QRandomGenerator>

namespace ume::dashboard {

MockBackend::MockBackend(QObject* parent) : QObject(parent) {
    connect(&timer_, &QTimer::timeout, this, &MockBackend::tick);
    timer_.start(1000); // tick every second
}

void MockBackend::tick() {
    // Walk telemetry metrics slightly
    auto* rng = QRandomGenerator::global();
    vram_usage_mb_ += rng->bounded(-100.0, 105.0);
    ram_usage_mb_ += rng->bounded(-50.0, 55.0);
    ssd_usage_mb_ += rng->bounded(-10.0, 12.0);

    // Keep within reasonable ranges
    vram_usage_mb_ = qBound(1000.0, vram_usage_mb_, 16384.0);
    ram_usage_mb_ = qBound(2000.0, ram_usage_mb_, 32768.0);
    ssd_usage_mb_ = qBound(0.0, ssd_usage_mb_, 100000.0);

    migration_rate_ = qMax(0.0, migration_rate_ + rng->bounded(-0.2, 0.25));
    system_health_ = qBound(70.0, system_health_ + rng->bounded(-0.5, 0.5), 100.0);
    prediction_accuracy_ = qBound(0.80, prediction_accuracy_ + rng->bounded(-0.01, 0.01), 0.99);
    simulation_accuracy_ = qBound(0.85, simulation_accuracy_ + rng->bounded(-0.01, 0.01), 0.99);

    emit telemetry_updated();
}

std::vector<AppMetric> MockBackend::get_applications() const {
    return {{"PyTorch NLP Training", 8192.0, 12288.0, 20480.0, 8.5, 42, 94.5},
            {"TensorFlow ResNet50", 4096.0, 8192.0, 0.0, 4.2, 12, 98.0},
            {"C++ Vector Database", 512.0, 4096.0, 24576.0, 1.8, 8, 92.3}};
}

std::vector<UIRecommendation> MockBackend::get_recommendations() const {
    return {
        {"Memory Health", "CRITICAL", "VRAM pressure exceeds critical limit.",
         "PyTorch NLP training active allocation demands exceed 90% peak VRAM capacity.",
         "Enable VRAM-to-RAM eviction policies or migrate cold weight layers.", 2500, 0.95, 85.0},
        {"Performance Optimization", "WARNING", "High redundant migration thrashing detected.",
         "Object ID 42 migrated between RAM and VRAM 8 times in past 30 seconds.",
         "Increase hysteresis thresholds or pin Object ID 42 temporarily.", 1200, 0.88, 92.5}};
}

std::vector<UISimulationPlan> MockBackend::get_simulation_plans() const {
    return {{1, "Plan A: Promote NLP Weights to VRAM", "VRAM", 12, 250.0, 0.98, 91.5},
            {2, "Plan B: Retain NLP Weights in RAM", "RAM", 85, 45.0, 0.85, 78.3},
            {3, "Plan C: Spill Weights to NVMe SSD", "SSD", 4200, 3.2, 0.45, 34.0}};
}

} // namespace ume::dashboard
