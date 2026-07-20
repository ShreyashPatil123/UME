#pragma once

/// @file memory_placement.h
/// @brief Placement Planner and Validator for memory scheduling decisions.
///
/// Part of UME Milestone M4 (Task T009).

#include "ume/event/memory_scheduler.h"
#include "ume/status.h"
#include "ume/types.h"

#include <cstdint>
#include <deque>
#include <mutex>
#include <unordered_set>
#include <vector>

namespace ume::event {

/// @brief Executable placement plan derived from a SchedulingDecision.
struct PlacementPlan {
    uint64_t plan_id{0};
    ObjectId object_id{ObjectId::kNull};
    TierClass source_tier{TierClass::kRam};
    TierClass target_tier{TierClass::kRam};
    uint64_t size_bytes{0};
    bool is_valid{false};
};

/// @brief Queued placement request item.
struct PlacementRequest {
    uint64_t request_id{0};
    SchedulingDecision decision{};
    Timestamp request_timestamp{Timestamp::kZero};
};

/// @brief Placement Planner, Queue, and Capacity Validator engine.
class MemoryPlacementEngine {
public:
    MemoryPlacementEngine(uint64_t max_ram_capacity_bytes, uint64_t max_vram_capacity_bytes,
                          uint64_t max_ssd_capacity_bytes) noexcept;
    ~MemoryPlacementEngine() = default;

    // Disable copy
    MemoryPlacementEngine(const MemoryPlacementEngine&) = delete;
    MemoryPlacementEngine& operator=(const MemoryPlacementEngine&) = delete;

    /// @brief Validates capacity and builds a PlacementPlan for a SchedulingDecision.
    [[nodiscard]] Result<PlacementPlan> plan_placement(const SchedulingDecision& decision) noexcept;

    /// @brief Validates and enqueues a PlacementRequest. Prevents duplicate ObjectIds in queue.
    Result<uint64_t> enqueue_request(const SchedulingDecision& decision) noexcept;

    /// @brief Dequeues the next pending PlacementRequest.
    Result<PlacementRequest> dequeue_request() noexcept;

    /// @brief Returns the count of pending queued placement requests.
    [[nodiscard]] size_t queue_size() const noexcept;

    /// @brief Clears all pending queued placement requests.
    void clear_queue() noexcept;

private:
    uint64_t max_ram_{0};
    uint64_t max_vram_{0};
    uint64_t max_ssd_{0};

    uint64_t current_ram_{0};
    uint64_t current_vram_{0};
    uint64_t current_ssd_{0};

    mutable std::mutex mutex_;
    std::deque<PlacementRequest> queue_;
    std::unordered_set<uint64_t> pending_objects_;
    uint64_t request_id_counter_{0};
    uint64_t plan_id_counter_{0};
};

} // namespace ume::event
