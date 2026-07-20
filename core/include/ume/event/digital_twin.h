#pragma once

/// @file digital_twin.h
/// @brief DigitalTwin simulation engine comparing candidate execution plans.
///
/// Part of UME Milestone M7 (Task T015).

#include "ume/event/memory_statistics.h"
#include "ume/status.h"
#include "ume/types.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

namespace ume::event {

/// @brief Profile details of a candidate execution plan.
struct SimulationPlan {
    uint64_t plan_id{0};
    std::string name;
    ObjectId object_id{ObjectId::kNull};
    TierClass target_tier{TierClass::kRam};
    uint64_t size_bytes{0};

    // Estimated projections
    uint64_t estimated_latency_us{0};
    uint64_t estimated_bandwidth_bytes_sec{0};
    double cache_hit_ratio{1.0};
    double overall_score{0.0}; ///< Higher is better (0.0 ↔ 100.0)
};

/// @brief Output result of running a Digital Twin simulation.
struct SimulationResult {
    std::vector<SimulationPlan> evaluated_plans;
    uint64_t selected_plan_id{0};
    double selected_plan_score{0.0};
};

/// @brief Simulator Context representing tier capacities.
struct SimulationContext {
    uint64_t current_ram_usage{0};
    uint64_t current_vram_usage{0};
    uint64_t current_ssd_usage{0};

    uint64_t max_ram_bytes{0};
    uint64_t max_vram_bytes{0};
    uint64_t max_ssd_bytes{0};
};

/// @brief Deterministic digital twin engine simulating memory movements.
class DigitalTwinEngine {
public:
    struct Metrics {
        std::atomic<uint64_t> simulations_executed{0};
        std::atomic<uint64_t> total_simulation_latency_ns{0};
        std::atomic<uint64_t> decision_quality_scores_sum{0};
    };

    DigitalTwinEngine(const StatisticsCollector& stats) noexcept;
    ~DigitalTwinEngine() = default;

    // Disable copy
    DigitalTwinEngine(const DigitalTwinEngine&) = delete;
    DigitalTwinEngine& operator=(const DigitalTwinEngine&) = delete;

    /// @brief Simulates and selects the highest scoring plan from candidate plans.
    Result<SimulationResult> simulate_strategies(
        ObjectId object_id, uint64_t size_bytes, const SimulationContext& context,
        const std::vector<SimulationPlan>& candidate_plans) noexcept;

    /// @brief Obtains metrics telemetry.
    [[nodiscard]] const Metrics& metrics() const noexcept { return metrics_; }

private:
    void evaluate_plan(SimulationPlan& plan, const SimulationContext& ctx) const noexcept;

    const StatisticsCollector& stats_;
    mutable std::mutex mutex_;
    Metrics metrics_{};
};

} // namespace ume::event
