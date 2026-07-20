/// @file memory_scheduler.cpp
/// @brief Implementation of MemoryScheduler tier placement decision engine.

#include "ume/event/memory_scheduler.h"

#include "ume/platform/clock.h"

namespace ume::event {

MemoryScheduler::MemoryScheduler(const StatisticsCollector& stats, const EventAnalyzer& analyzer,
                                 SchedulingPolicy policy) noexcept
    : stats_(stats), analyzer_(analyzer), policy_(policy) {}

SchedulingDecision MemoryScheduler::schedule_allocation(ObjectId object_id, uint64_t size_bytes,
                                                        uint32_t access_count,
                                                        EventPriority priority) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    SchedulingDecision decision{};
    decision.object_id = object_id;
    decision.allocation_size_bytes = size_bytes;
    decision.decision_timestamp = platform::monotonic_now_ns();
    decision.priority_score = static_cast<uint32_t>(priority) * 50 + 50;

    const MemoryStatisticsSnapshot snap = stats_.snapshot();

    // Check VRAM suitability (hot access or critical priority)
    if (access_count >= policy_.vram_hot_access_count_threshold ||
        priority == EventPriority::kCritical) {
        // Check VRAM capacity pressure
        if (snap.peak_vram_bytes > 0 && static_cast<double>(snap.current_vram_bytes + size_bytes) /
                                                static_cast<double>(snap.peak_vram_bytes) >
                                            policy_.vram_pressure_threshold) {
            decision.preferred_tier = TierClass::kRam;
            decision.reason = DecisionReason::kVramCapacityExceededRamFallback;
        } else {
            decision.preferred_tier = TierClass::kVram;
            decision.reason = DecisionReason::kHotAccessVram;
            decision.priority_score += 50;
        }
        return decision;
    }

    // Check RAM pressure threshold for SSD Spill
    if (snap.peak_ram_bytes > 0 && static_cast<double>(snap.current_ram_bytes + size_bytes) /
                                           static_cast<double>(snap.peak_ram_bytes) >
                                       policy_.ram_pressure_threshold) {
        decision.preferred_tier = TierClass::kNvme;
        decision.reason = DecisionReason::kRamThresholdExceededSsdSpill;
        return decision;
    }

    // Default RAM placement
    decision.preferred_tier = TierClass::kRam;
    decision.reason = DecisionReason::kDefaultPlacement;
    return decision;
}

void MemoryScheduler::update_policy(SchedulingPolicy policy) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    policy_ = policy;
}

SchedulingPolicy MemoryScheduler::policy() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return policy_;
}

} // namespace ume::event
