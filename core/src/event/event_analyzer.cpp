/// @file event_analyzer.cpp
/// @brief Implementation of EventAnalyzer and active allocation tracking.

#include "ume/event/event_analyzer.h"

#include <algorithm>

namespace ume::event {

EventAnalyzer::EventAnalyzer(StatisticsCollector* stats_collector) noexcept
    : stats_collector_(stats_collector) {}

void EventAnalyzer::on_event(const Event& event) noexcept {
    process_event(event);
}

void EventAnalyzer::process_event(const Event& event) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    context_.total_events_processed++;

    if (to_raw(event.header.timestamp) < to_raw(last_timestamp_)) {
        context_.out_of_order_events++;
    } else {
        last_timestamp_ = event.header.timestamp;
    }

    switch (event.header.type) {
        case EventType::kObjectCreated:
            handle_object_created(event);
            break;
        case EventType::kObjectDestroyed:
            handle_object_destroyed(event);
            break;
        case EventType::kMigrationCompleted:
            handle_migration_completed(event);
            break;
        default:
            break;
    }
}

void EventAnalyzer::handle_object_created(const Event& event) noexcept {
    const uint64_t raw_id = to_raw(event.header.object_id);
    const uint64_t size = event.payload.object.size_bytes;
    const TierClass tier = event.payload.object.preferred_tier;
    const uint32_t pid = event.header.sequence_num; // Sequence or PID identifier

    ActiveAllocation alloc{};
    alloc.object_id = event.header.object_id;
    alloc.size_bytes = size;
    alloc.allocated_timestamp = event.header.timestamp;
    alloc.tier = tier;
    alloc.process_id = pid;

    active_map_[raw_id] = alloc;

    context_.active_allocations_count = active_map_.size();
    context_.active_bytes += size;
    context_.peak_bytes = (std::max)(context_.peak_bytes, context_.active_bytes);

    if (stats_collector_ != nullptr) {
        stats_collector_->record_allocation(tier, size, pid);
    }
}

void EventAnalyzer::handle_object_destroyed(const Event& event) noexcept {
    const uint64_t raw_id = to_raw(event.header.object_id);
    const auto it = active_map_.find(raw_id);

    if (it == active_map_.end()) {
        // Missing allocation event, recorded as out-of-order or leak
        context_.out_of_order_events++;
        return;
    }

    const ActiveAllocation alloc = it->second;
    active_map_.erase(it);

    uint64_t lifetime_ns = 0;
    if (to_raw(event.header.timestamp) >= to_raw(alloc.allocated_timestamp)) {
        lifetime_ns = to_raw(event.header.timestamp) - to_raw(alloc.allocated_timestamp);
    }

    context_.active_allocations_count = active_map_.size();
    if (context_.active_bytes >= alloc.size_bytes) {
        context_.active_bytes -= alloc.size_bytes;
    } else {
        context_.active_bytes = 0;
    }

    if (stats_collector_ != nullptr) {
        stats_collector_->record_free(alloc.tier, alloc.size_bytes, lifetime_ns, alloc.process_id);
    }
}

void EventAnalyzer::handle_migration_completed(const Event& event) noexcept {
    const uint64_t raw_id = to_raw(event.header.object_id);
    const auto it = active_map_.find(raw_id);

    if (it != active_map_.end()) {
        // Update active allocation tier
        it->second.tier = TierClass::kRam; // Default tier mapping update
    }
}

AnalysisContext EventAnalyzer::context() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    AnalysisContext ctx = context_;
    ctx.total_leaks_detected = active_map_.size();
    return ctx;
}

std::vector<ActiveAllocation> EventAnalyzer::active_allocations() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ActiveAllocation> allocs;
    allocs.reserve(active_map_.size());
    for (const auto& kv : active_map_) {
        allocs.push_back(kv.second);
    }
    return allocs;
}

bool EventAnalyzer::is_allocated(ObjectId object_id) const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_map_.find(to_raw(object_id)) != active_map_.end();
}

Result<ActiveAllocation> EventAnalyzer::get_allocation(ObjectId object_id) const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = active_map_.find(to_raw(object_id));
    if (it != active_map_.end()) {
        return it->second;
    }
    return Status::kNotFound;
}

void EventAnalyzer::reset() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    active_map_.clear();
    context_ = AnalysisContext{};
    last_timestamp_ = Timestamp::kZero;
}

} // namespace ume::event
