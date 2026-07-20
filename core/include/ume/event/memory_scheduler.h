#pragma once

/// @file memory_scheduler.h
/// @brief Memory Scheduler decision engine for tier placement (VRAM, RAM, SSD Spill).
///
/// Part of UME Milestone M4 (Task T008).

#include "ume/event/event_analyzer.h"
#include "ume/event/memory_statistics.h"
#include "ume/status.h"
#include "ume/types.h"

#include <cstdint>
#include <mutex>

namespace ume::event {

/// @brief Explicit justification reason for a memory scheduling decision.
enum class DecisionReason : uint8_t {
    kDefaultPlacement = 0,
    kHotAccessVram = 1,
    kRamThresholdExceededSsdSpill = 2,
    kVramCapacityExceededRamFallback = 3,
    kLowPriorityColdAllocation = 4,
};

/// @brief Configurable threshold policy settings for the scheduler.
struct SchedulingPolicy {
    double ram_pressure_threshold{0.85};          ///< 85% RAM usage triggers SSD spill
    double vram_pressure_threshold{0.90};         ///< 90% VRAM usage triggers RAM fallback
    uint32_t vram_hot_access_count_threshold{10}; ///< Access count threshold for VRAM promotion
    uint64_t large_allocation_threshold_bytes{1024 * 1024 * 1024}; ///< 1 GB threshold
};

/// @brief Outcome decision emitted by the MemoryScheduler.
struct SchedulingDecision {
    ObjectId object_id{ObjectId::kNull};
    TierClass preferred_tier{TierClass::kRam};
    uint32_t priority_score{100};
    DecisionReason reason{DecisionReason::kDefaultPlacement};
    Timestamp decision_timestamp{Timestamp::kZero};
    uint64_t allocation_size_bytes{0};
};

/// @brief Decision engine for determining optimal memory tier placements.
class MemoryScheduler {
public:
    MemoryScheduler(const StatisticsCollector& stats, const EventAnalyzer& analyzer,
                    SchedulingPolicy policy = {}) noexcept;
    ~MemoryScheduler() = default;

    // Disable copy
    MemoryScheduler(const MemoryScheduler&) = delete;
    MemoryScheduler& operator=(const MemoryScheduler&) = delete;

    /// @brief Computes optimal memory tier placement decision for an allocation.
    [[nodiscard]] SchedulingDecision schedule_allocation(
        ObjectId object_id, uint64_t size_bytes, uint32_t access_count = 0,
        EventPriority priority = EventPriority::kNormal) noexcept;

    /// @brief Updates scheduler policy settings.
    void update_policy(SchedulingPolicy policy) noexcept;

    /// @brief Obtains active scheduler policy settings.
    [[nodiscard]] SchedulingPolicy policy() const noexcept;

private:
    const StatisticsCollector& stats_;
    const EventAnalyzer& analyzer_;
    mutable std::mutex mutex_;
    SchedulingPolicy policy_;
};

} // namespace ume::event
