#pragma once

/// @file event_analyzer.h
/// @brief EventAnalyzer consuming replayed events and reconstructing application memory behavior.
///
/// Part of UME Milestone M3 (Task T006).

#include "ume/event/event.h"
#include "ume/event/event_dispatcher.h"
#include "ume/event/memory_statistics.h"
#include "ume/status.h"
#include "ume/types.h"

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace ume::event {

/// @brief Data recorded for an active open allocation.
struct ActiveAllocation {
    ObjectId object_id{ObjectId::kNull};
    uint64_t size_bytes{0};
    Timestamp allocated_timestamp{Timestamp::kZero};
    TierClass tier{TierClass::kRam};
    uint32_t process_id{0};
    uint32_t thread_id{0};
};

/// @brief Overall state and context of event analysis.
struct AnalysisContext {
    uint64_t total_events_processed{0};
    uint64_t active_allocations_count{0};
    uint64_t active_bytes{0};
    uint64_t peak_bytes{0};
    uint64_t total_leaks_detected{0};
    uint64_t out_of_order_events{0};
};

/// @brief Analyzes event streams to reconstruct active allocations, lifetimes, leaks, and memory
/// behavior.
class EventAnalyzer : public IEventListener {
public:
    explicit EventAnalyzer(StatisticsCollector* stats_collector = nullptr) noexcept;
    ~EventAnalyzer() override = default;

    // Disable copy
    EventAnalyzer(const EventAnalyzer&) = delete;
    EventAnalyzer& operator=(const EventAnalyzer&) = delete;

    /// @brief Callback invoked when a subscribed event is received from EventDispatcher.
    void on_event(const Event& event) noexcept override;

    /// @brief Manually processes a single event.
    void process_event(const Event& event) noexcept;

    /// @brief Obtains current analysis context summary.
    [[nodiscard]] AnalysisContext context() const noexcept;

    /// @brief Obtains active allocations table.
    [[nodiscard]] std::vector<ActiveAllocation> active_allocations() const noexcept;

    /// @brief Checks if a specific ObjectId is currently allocated.
    [[nodiscard]] bool is_allocated(ObjectId object_id) const noexcept;

    /// @brief Returns pointer to active allocation if present.
    [[nodiscard]] Result<ActiveAllocation> get_allocation(ObjectId object_id) const noexcept;

    /// @brief Resets analyzer state.
    void reset() noexcept;

private:
    void handle_object_created(const Event& event) noexcept;
    void handle_object_destroyed(const Event& event) noexcept;
    void handle_migration_completed(const Event& event) noexcept;

    mutable std::mutex mutex_;
    StatisticsCollector* stats_collector_{nullptr};
    std::unordered_map<uint64_t, ActiveAllocation> active_map_;
    AnalysisContext context_{};
    Timestamp last_timestamp_{Timestamp::kZero};
};

} // namespace ume::event
