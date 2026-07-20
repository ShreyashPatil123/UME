/// @file prefetch_engine.cpp
/// @brief Implementation of locality-aware Intelligent Prefetch Engine.

#include "ume/event/prefetch_engine.h"

#include "ume/platform/clock.h"

#include <algorithm>
#include <chrono>

namespace ume::event {

PrefetchEngine::PrefetchEngine(MemoryPlacementEngine& placement) noexcept : placement_(placement) {}

PrefetchEngine::~PrefetchEngine() {
    (void)stop();
}

Result<void> PrefetchEngine::start() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    if (is_running_) {
        return Status::kAlreadyInitialized;
    }

    is_running_ = true;
    worker_thread_ = std::thread(&PrefetchEngine::worker_loop, this);
    return Status::kOk;
}

Result<void> PrefetchEngine::stop() noexcept {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!is_running_) {
            return Status::kOk;
        }
        is_running_ = false;
    }

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    return Status::kOk;
}

void PrefetchEngine::record_access(ObjectId object_id, uint64_t virtual_address) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    const uint64_t raw_id = to_raw(object_id);

    auto& history = access_history_[raw_id];
    history.push_back(virtual_address);
    if (history.size() > 5) {
        history.erase(history.begin());
    }

    // Locality and pattern recognition analysis
    analyze_locality(object_id, virtual_address);
}

void PrefetchEngine::analyze_locality(ObjectId object_id, uint64_t address) noexcept {
    (void)address;
    const uint64_t raw_id = to_raw(object_id);
    const auto& history = access_history_[raw_id];
    if (history.size() < 3) {
        return;
    }

    // Sequential Access Detection: check stride delta
    const int64_t stride1 = static_cast<int64_t>(history[history.size() - 1]) -
                            static_cast<int64_t>(history[history.size() - 2]);
    const int64_t stride2 = static_cast<int64_t>(history[history.size() - 2]) -
                            static_cast<int64_t>(history[history.size() - 3]);

    if (stride1 == stride2 && stride1 != 0) {
        // Sequential stride detected! Create prefetch request
        PrefetchRequest req{};
        req.request_id = ++request_id_counter_;
        req.object_id = object_id;
        req.target_tier = TierClass::kVram; // Target fast memory tier
        req.confidence = 0.85;
        req.priority = 180;
        req.expiration = static_cast<Timestamp>(to_raw(platform::monotonic_now_ns()) +
                                                1000000000ULL); // 1s expiration
        req.estimated_benefit_us = 120;
        req.estimated_cost_us = 20;

        prefetch_queue_.push_back(req);
    }
}

Result<void> PrefetchEngine::submit_prefetches(
    const std::vector<PrefetchRequest>& requests) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& req : requests) {
        PrefetchRequest item = req;
        item.request_id = ++request_id_counter_;
        prefetch_queue_.push_back(item);
    }
    return Status::kOk;
}

void PrefetchEngine::cancel_prefetch(ObjectId object_id) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    const auto it = std::remove_if(
        prefetch_queue_.begin(), prefetch_queue_.end(),
        [object_id](const PrefetchRequest& req) { return req.object_id == object_id; });
    if (it != prefetch_queue_.end()) {
        const size_t cancelled_count = std::distance(it, prefetch_queue_.end());
        metrics_.requests_cancelled.fetch_add(cancelled_count, std::memory_order_relaxed);
        prefetch_queue_.erase(it, prefetch_queue_.end());
    }
}

size_t PrefetchEngine::queue_size() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return prefetch_queue_.size();
}

void PrefetchEngine::worker_loop() noexcept {
    while (is_running_.load(std::memory_order_relaxed)) {
        PrefetchRequest req{};
        bool has_work = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!prefetch_queue_.empty()) {
                req = prefetch_queue_.front();
                prefetch_queue_.pop_front();
                has_work = true;
            }
        }

        if (has_work) {
            // Check request expiration
            const auto now = platform::monotonic_now_ns();
            if (to_raw(req.expiration) > 0 && to_raw(now) > to_raw(req.expiration)) {
                metrics_.requests_cancelled.fetch_add(1, std::memory_order_relaxed);
                continue;
            }

            // Route request to placement engine
            SchedulingDecision dec{};
            dec.object_id = req.object_id;
            dec.preferred_tier = req.target_tier;
            dec.allocation_size_bytes = 4096; // Standard standard size
            dec.priority_score = req.priority;

            const auto plan_res = placement_.enqueue_request(dec);
            if (plan_res.ok()) {
                metrics_.requests_issued.fetch_add(1, std::memory_order_relaxed);
                metrics_.prefetch_hits.fetch_add(1, std::memory_order_relaxed);
                metrics_.total_confidence_sum_scaled.fetch_add(
                    static_cast<uint64_t>(req.confidence * 1000.0), std::memory_order_relaxed);
            } else {
                metrics_.prefetch_misses.fetch_add(1, std::memory_order_relaxed);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

} // namespace ume::event
