/// @file digital_twin.cpp
/// @brief Implementation of DigitalTwinEngine deterministic candidate strategy simulation.

#include "ume/event/digital_twin.h"

#include "ume/platform/clock.h"

#include <algorithm>

namespace ume::event {

DigitalTwinEngine::DigitalTwinEngine(const StatisticsCollector& stats) noexcept : stats_(stats) {}

Result<SimulationResult> DigitalTwinEngine::simulate_strategies(
    ObjectId object_id, uint64_t size_bytes, const SimulationContext& context,
    const std::vector<SimulationPlan>& candidate_plans) noexcept {
    const auto start_time = platform::monotonic_now_ns();
    std::lock_guard<std::mutex> lock(mutex_);

    if (candidate_plans.empty() || object_id == ObjectId::kNull) {
        return Status::kInvalidArgument;
    }

    SimulationResult result{};
    result.evaluated_plans = candidate_plans;

    double best_score = -1.0;
    uint64_t best_plan_id = 0;

    for (auto& plan : result.evaluated_plans) {
        plan.object_id = object_id;
        plan.size_bytes = size_bytes;

        // Perform deterministic evaluation
        evaluate_plan(plan, context);

        if (plan.overall_score > best_score) {
            best_score = plan.overall_score;
            best_plan_id = plan.plan_id;
        }
    }

    result.selected_plan_id = best_plan_id;
    result.selected_plan_score = best_score;

    metrics_.simulations_executed.fetch_add(1, std::memory_order_relaxed);
    metrics_.decision_quality_scores_sum.fetch_add(static_cast<uint64_t>(best_score),
                                                   std::memory_order_relaxed);

    const auto duration = to_raw(platform::monotonic_now_ns()) - to_raw(start_time);
    metrics_.total_simulation_latency_ns.fetch_add(duration, std::memory_order_relaxed);

    return result;
}

void DigitalTwinEngine::evaluate_plan(SimulationPlan& plan,
                                      const SimulationContext& ctx) const noexcept {
    // Estimators logic
    double latency_score = 100.0;
    double bandwidth_score = 100.0;
    double capacity_penalty = 0.0;

    switch (plan.target_tier) {
        case TierClass::kVram:
            // High VRAM bandwidth (HBM/GDDR), low latency
            plan.estimated_latency_us = 10;
            plan.estimated_bandwidth_bytes_sec = 250ULL * 1024ULL * 1024ULL * 1024ULL; // 250 GB/s
            plan.cache_hit_ratio = 0.98;

            // Capacity check penalty
            if (ctx.max_vram_bytes > 0 &&
                ctx.current_vram_usage + plan.size_bytes > ctx.max_vram_bytes) {
                capacity_penalty = 80.0; // severe penalty if over capacity
            }
            break;

        case TierClass::kRam:
            // Moderate RAM bandwidth, moderate latency
            plan.estimated_latency_us = 80;
            plan.estimated_bandwidth_bytes_sec = 50ULL * 1024ULL * 1024ULL * 1024ULL; // 50 GB/s
            plan.cache_hit_ratio = 0.85;

            if (ctx.max_ram_bytes > 0 &&
                ctx.current_ram_usage + plan.size_bytes > ctx.max_ram_bytes) {
                capacity_penalty = 80.0;
            }
            break;

        case TierClass::kNvme:
            // Higher NVMe latency, lower bandwidth
            plan.estimated_latency_us = 5000;                                        // 5 ms
            plan.estimated_bandwidth_bytes_sec = 3ULL * 1024ULL * 1024ULL * 1024ULL; // 3 GB/s
            plan.cache_hit_ratio = 0.50;

            if (ctx.max_ssd_bytes > 0 &&
                ctx.current_ssd_usage + plan.size_bytes > ctx.max_ssd_bytes) {
                capacity_penalty = 80.0;
            }
            break;

        default:
            plan.estimated_latency_us = 10000;
            plan.estimated_bandwidth_bytes_sec = 1ULL * 1024ULL * 1024ULL * 1024ULL;
            plan.cache_hit_ratio = 0.10;
            break;
    }

    // Map latency/bandwidth to standardized 0-100 scores
    if (plan.estimated_latency_us > 10000)
        latency_score = 10.0;
    else
        latency_score = 100.0 - (static_cast<double>(plan.estimated_latency_us) / 110.0);

    bandwidth_score = (std::min)(100.0, static_cast<double>(plan.estimated_bandwidth_bytes_sec) /
                                            (3ULL * 1024ULL * 1024ULL * 1024ULL));

    // Weighted average overall score calculation
    double score = (latency_score * 0.4) + (bandwidth_score * 0.3) + (plan.cache_hit_ratio * 30.0);
    plan.overall_score = (std::max)(0.0, score - capacity_penalty);
}

} // namespace ume::event
