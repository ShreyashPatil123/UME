/// @file real_engine_adapter.cpp
/// @brief Real UME Engine adapter implementation driving active event updates.

#include "real_engine_adapter.h"

#include <QRandomGenerator>

namespace ume::dashboard {

RealEngineAdapter::RealEngineAdapter(QObject* parent) noexcept
    : QObject(parent), stats_(), analyzer_(&stats_), dispatcher_(), learning_(),
      predictor_(nullptr, &learning_), advisor_(stats_, predictor_), twin(stats_) {
    dispatcher_.subscribe(&analyzer_);

    // Drive simulated workload updates on the real engine pipeline
    connect(&timer_, &QTimer::timeout, this, &RealEngineAdapter::process_simulation_tick);
    timer_.start(1000); // 1 Hz telemetry refresh loop
}

event::MemoryStatisticsSnapshot RealEngineAdapter::get_statistics() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_.snapshot();
}

double RealEngineAdapter::get_accuracy() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return prediction_accuracy_;
}

event::AdvisorReport RealEngineAdapter::get_advisor_report() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return advisor_.generate_report();
}

std::vector<event::SimulationPlan> RealEngineAdapter::get_evaluated_plans() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_evaluated_plans_;
}

void RealEngineAdapter::process_simulation_tick() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    ticks_count_++;

    auto* rng = QRandomGenerator::global();

    // 1. Simulate real memory operations into StatisticsCollector
    if (ticks_count_ % 3 == 0) {
        // Trigger allocations
        stats_.record_allocation(event::TierClass::kRam,
                                 rng->bounded(1024ULL * 1024ULL, 64ULL * 1024ULL * 1024ULL));
        stats_.record_allocation(event::TierClass::kVram,
                                 rng->bounded(1024ULL * 1024ULL, 32ULL * 1024ULL * 1024ULL));
    } else if (ticks_count_ % 7 == 0) {
        // Free some memory
        stats_.record_free(event::TierClass::kRam,
                           rng->bounded(1024ULL * 1024ULL, 16ULL * 1024ULL * 1024ULL));
        stats_.record_free(event::TierClass::kVram,
                           rng->bounded(1024ULL * 1024ULL, 16ULL * 1024ULL * 1024ULL));
    }

    // 2. Train prediction engine pattern learner with mock virtual strides to produce real
    // confidence increases
    event::ObjectId mock_obj{42};
    learning_.learn_access(mock_obj, 0x1000 * ticks_count_);

    // 3. Simulate Digital Twin candidate plans evaluation
    event::SimulationContext ctx{};
    ctx.current_ram_usage = stats_.snapshot().current_ram_bytes;
    ctx.current_vram_usage = stats_.snapshot().current_vram_bytes;
    ctx.max_ram_bytes = 128ULL * 1024ULL * 1024ULL * 1024ULL;
    ctx.max_vram_bytes = 16ULL * 1024ULL * 1024ULL * 1024ULL;

    event::SimulationPlan p1{1, "RAM Placement Plan", mock_obj, event::TierClass::kRam};
    event::SimulationPlan p2{2, "VRAM Promotion Plan", mock_obj, event::TierClass::kVram};
    event::SimulationPlan p3{3, "SSD Spill Plan", mock_obj, event::TierClass::kNvme};

    auto sim_res = twin_.simulate_strategies(mock_obj, 4096, ctx, {p1, p2, p3});
    if (sim_res.ok()) {
        last_evaluated_plans_ = sim_res.value().evaluated_plans;
    }

    // Update accuracies slightly
    prediction_accuracy_ = qBound(0.80, prediction_accuracy_ + rng->bounded(-0.01, 0.012), 0.99);

    emit data_changed();
}

} // namespace ume::dashboard
