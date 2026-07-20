#pragma once

/// @file memory_optimizer.h
/// @brief Asynchronous tier residency optimizer, memory aging, and cost estimation.
///
/// Part of UME Milestone M5 (Task T010).

#include "ume/event/memory_placement.h"
#include "ume/event/memory_scheduler.h"
#include "ume/event/memory_statistics.h"
#include "ume/status.h"
#include "ume/types.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ume::event {

/// @brief Residency and access temperature tracking metadata for an object.
struct ResidencyInfo {
    ObjectId object_id{ObjectId::kNull};
    TierClass current_tier{TierClass::kRam};
    Timestamp last_access_time{Timestamp::kZero};
    uint32_t access_count{0};
    double temperature{50.0}; ///< Hot (100.0) ↔ Cold (0.0)
    uint64_t size_bytes{0};
};

/// @brief Executable memory migration plan task.
struct MigrationTask {
    uint64_t task_id{0};
    ObjectId object_id{ObjectId::kNull};
    TierClass source_tier{TierClass::kRam};
    TierClass target_tier{TierClass::kRam};
    uint64_t size_bytes{0};
    double cost_estimate_us{0.0};
    uint32_t priority{100};
};

/// @brief Asynchronous memory optimizer background engine.
class MemoryOptimizer {
public:
    struct Metrics {
        std::atomic<uint64_t> migrations_attempted{0};
        std::atomic<uint64_t> migrations_completed{0};
        std::atomic<uint64_t> migrations_cancelled{0};
        std::atomic<uint64_t> total_migration_time_us{0};
        std::atomic<uint64_t> total_bandwidth_used_bytes{0};
    };

    MemoryOptimizer(StatisticsCollector& stats, MemoryPlacementEngine& placement,
                    uint64_t max_migration_bandwidth_bytes_sec = 10ULL * 1024ULL * 1024ULL *
                                                                 1024ULL) noexcept;
    ~MemoryOptimizer();

    // Disable copy
    MemoryOptimizer(const MemoryOptimizer&) = delete;
    MemoryOptimizer& operator=(const MemoryOptimizer&) = delete;

    /// @brief Starts background optimization worker thread loop.
    Result<void> start() noexcept;

    /// @brief Stops background optimization worker thread loop.
    Result<void> stop() noexcept;

    /// @brief Explicitly records an access observation event to update object temperatures.
    void record_access(ObjectId object_id, uint64_t size_bytes, TierClass tier) noexcept;

    /// @brief Triggers an on-demand pass of residency check and migration scheduling.
    void optimize_pass() noexcept;

    /// @brief Returns residency status info of an object.
    [[nodiscard]] Result<ResidencyInfo> get_residency(ObjectId object_id) const noexcept;

    /// @brief Returns active telemetry metrics.
    [[nodiscard]] const Metrics& metrics() const noexcept { return metrics_; }

    /// @brief Sets temperature decay rate per optimization pass (e.g. 0.05 = 5%).
    void set_decay_rate(double decay_rate) noexcept;

private:
    void worker_loop() noexcept;
    double estimate_migration_cost(TierClass src, TierClass dst, uint64_t size) const noexcept;

    StatisticsCollector& stats_;
    MemoryPlacementEngine& placement_;
    uint64_t max_bandwidth_sec_{0};
    double decay_rate_{0.05};

    mutable std::mutex mutex_;
    std::unordered_map<uint64_t, ResidencyInfo> residency_map_;
    std::vector<MigrationTask> pending_migrations_;
    uint64_t task_id_counter_{0};

    std::thread worker_thread_;
    std::atomic<bool> is_running_{false};
    Metrics metrics_{};
};

} // namespace ume::event
