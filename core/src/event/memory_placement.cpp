/// @file memory_placement.cpp
/// @brief Implementation of Placement Planner and Capacity Validator engine.

#include "ume/event/memory_placement.h"

#include "ume/platform/clock.h"

namespace ume::event {

MemoryPlacementEngine::MemoryPlacementEngine(uint64_t max_ram_capacity_bytes,
                                             uint64_t max_vram_capacity_bytes,
                                             uint64_t max_ssd_capacity_bytes) noexcept
    : max_ram_(max_ram_capacity_bytes), max_vram_(max_vram_capacity_bytes),
      max_ssd_(max_ssd_capacity_bytes) {}

Result<PlacementPlan> MemoryPlacementEngine::plan_placement(
    const SchedulingDecision& decision) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    if (decision.object_id == ObjectId::kNull || decision.allocation_size_bytes == 0) {
        return Status::kInvalidArgument;
    }

    PlacementPlan plan{};
    plan.plan_id = ++plan_id_counter_;
    plan.object_id = decision.object_id;
    plan.source_tier = TierClass::kRam;
    plan.target_tier = decision.preferred_tier;
    plan.size_bytes = decision.allocation_size_bytes;

    // Validate capacity
    switch (decision.preferred_tier) {
        case TierClass::kVram:
            if (current_vram_ + decision.allocation_size_bytes > max_vram_) {
                plan.is_valid = false;
                return Status::kCapacityExceeded;
            }
            break;
        case TierClass::kNvme:
            if (current_ssd_ + decision.allocation_size_bytes > max_ssd_) {
                plan.is_valid = false;
                return Status::kCapacityExceeded;
            }
            break;
        case TierClass::kRam:
        default:
            if (current_ram_ + decision.allocation_size_bytes > max_ram_) {
                plan.is_valid = false;
                return Status::kCapacityExceeded;
            }
            break;
    }

    plan.is_valid = true;
    return plan;
}

Result<uint64_t> MemoryPlacementEngine::enqueue_request(
    const SchedulingDecision& decision) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    if (decision.object_id == ObjectId::kNull) {
        return Status::kInvalidArgument;
    }

    const uint64_t raw_id = to_raw(decision.object_id);
    if (pending_objects_.find(raw_id) != pending_objects_.end()) {
        return Status::kAlreadyExists; // Duplicate request prevention
    }

    PlacementRequest req{};
    req.request_id = ++request_id_counter_;
    req.decision = decision;
    req.request_timestamp = platform::monotonic_now_ns();

    queue_.push_back(req);
    pending_objects_.insert(raw_id);

    return req.request_id;
}

Result<PlacementRequest> MemoryPlacementEngine::dequeue_request() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    if (queue_.empty()) {
        return Status::kNotFound;
    }

    PlacementRequest req = queue_.front();
    queue_.pop_front();
    pending_objects_.erase(to_raw(req.decision.object_id));

    return req;
}

size_t MemoryPlacementEngine::queue_size() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void MemoryPlacementEngine::clear_queue() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
    pending_objects_.clear();
}

} // namespace ume::event
