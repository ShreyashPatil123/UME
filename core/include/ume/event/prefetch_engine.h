#pragma once

/// @file prefetch_engine.h
/// @brief Confidence-based, locality-aware Intelligent Prefetch Engine.
///
/// Part of UME Milestone M5 (Task T011).

#include "ume/event/memory_placement.h"
#include "ume/event/memory_scheduler.h"
#include "ume/status.h"
#include "ume/types.h"

#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ume::event {

/// @brief Prefetch load request item.
struct PrefetchRequest {
    uint64_t request_id{0};
    ObjectId object_id{ObjectId::kNull};
    TierClass target_tier{TierClass::kRam};
    double confidence{0.5}; ///< Confidence scale (0.0 ↔ 1.0)
    uint32_t priority{100};
    Timestamp expiration{Timestamp::kZero};
    uint64_t estimated_benefit_us{0};
    uint64_t estimated_cost_us{0};
};

/// @brief Intelligent Prefetch Engine monitoring access patterns and scheduling loads.
class PrefetchEngine {
public:
    struct Metrics {
        std::atomic<uint64_t> requests_issued{0};
        std::atomic<uint64_t> requests_cancelled{0};
        std::atomic<uint64_t> prefetch_hits{0};
        std::atomic<uint64_t> prefetch_misses{0};
        std::atomic<uint64_t> total_confidence_sum_scaled{0}; ///< Scaled by 1000 for atomic updates
        std::atomic<uint64_t> total_queue_latency_us{0};
        std::atomic<uint64_t> bandwidth_consumed_bytes{0};
    };

    PrefetchEngine(MemoryPlacementEngine& placement) noexcept;
    ~PrefetchEngine();

    // Disable copy
    PrefetchEngine(const PrefetchEngine&) = delete;
    PrefetchEngine& operator=(const PrefetchEngine&) = delete;

    /// @brief Starts prefetch worker thread loop.
    Result<void> start() noexcept;

    /// @brief Stops prefetch worker thread loop.
    Result<void> stop() noexcept;

    /// @brief Records an access event observation to update prefetch history analysis.
    void record_access(ObjectId object_id, uint64_t virtual_address) noexcept;

    /// @brief Submits a batch of prefetch requests.
    Result<void> submit_prefetches(const std::vector<PrefetchRequest>& requests) noexcept;

    /// @brief Cancels all pending prefetch requests for a given ObjectId.
    void cancel_prefetch(ObjectId object_id) noexcept;

    /// @brief Returns metrics telemetry.
    [[nodiscard]] const Metrics& metrics() const noexcept { return metrics_; }

    /// @brief Returns size of current prefetch queue.
    [[nodiscard]] size_t queue_size() const noexcept;

private:
    void worker_loop() noexcept;
    void analyze_locality(ObjectId object_id, uint64_t address) noexcept;

    MemoryPlacementEngine& placement_;
    mutable std::mutex mutex_;
    std::deque<PrefetchRequest> prefetch_queue_;
    std::unordered_map<uint64_t, std::vector<uint64_t>>
        access_history_; ///< ObjectId -> last virtual addresses

    std::thread worker_thread_;
    std::atomic<bool> is_running_{false};
    uint64_t request_id_counter_{0};
    Metrics metrics_{};
};

} // namespace ume::event
