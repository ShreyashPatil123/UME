/// @file memory_optimizer.cpp
/// @brief Implementation of asynchronous tier residency optimizer and memory aging.

#include "ume/event/memory_optimizer.h"

#include "ume/platform/clock.h"

#include <chrono>

namespace ume::event {

MemoryOptimizer::MemoryOptimizer(StatisticsCollector& stats, MemoryPlacementEngine& placement,
                                 uint64_t max_migration_bandwidth_bytes_sec) noexcept
    : stats_(stats), placement_(placement), max_bandwidth_sec_(max_migration_bandwidth_bytes_sec) {}

MemoryOptimizer::~MemoryOptimizer() {
    (void)stop();
}

Result<void> MemoryOptimizer::start() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    if (is_running_) {
        return Status::kAlreadyInitialized;
    }

    is_running_ = true;
    worker_thread_ = std::thread(&MemoryOptimizer::worker_loop, this);
    return Status::kOk;
}

Result<void> MemoryOptimizer::stop() noexcept {
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

void MemoryOptimizer::record_access(ObjectId object_id, uint64_t size_bytes,
                                    TierClass tier) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    const uint64_t raw_id = to_raw(object_id);

    auto& res = residency_map_[raw_id];
    res.object_id = object_id;
    res.size_bytes = size_bytes;
    res.current_tier = tier;
    res.last_access_time = platform::monotonic_now_ns();
    res.access_count++;

    // Temperature promotion
    res.temperature = (std::min)(100.0, res.temperature + 10.0);
}

void MemoryOptimizer::optimize_pass() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);


    // 1. Age existing residency objects (temperature decay)
    for (auto& [raw_id, res] : residency_map_) {
        // Temperature decays over time
        res.temperature = (std::max)(0.0, res.temperature - (res.temperature * decay_rate_));

        // 2. Identify cold/hot placements and migration needs
        if (res.temperature < 20.0 && res.current_tier == TierClass::kVram) {
            // Cold object in VRAM -> Demote to RAM
            MigrationTask task{};
            task.task_id = ++task_id_counter_;
            task.object_id = res.object_id;
            task.source_tier = TierClass::kVram;
            task.target_tier = TierClass::kRam;
            task.size_bytes = res.size_bytes;
            task.cost_estimate_us =
                estimate_migration_cost(TierClass::kVram, TierClass::kRam, res.size_bytes);
            task.priority = 50;

            pending_migrations_.push_back(task);
        } else if (res.temperature > 80.0 && res.current_tier == TierClass::kRam) {
            // Hot object in RAM -> Promote to VRAM
            MigrationTask task{};
            task.task_id = ++task_id_counter_;
            task.object_id = res.object_id;
            task.source_tier = TierClass::kRam;
            task.target_tier = TierClass::kVram;
            task.size_bytes = res.size_bytes;
            task.cost_estimate_us =
                estimate_migration_cost(TierClass::kRam, TierClass::kVram, res.size_bytes);
            task.priority = 150;

            pending_migrations_.push_back(task);
        }
    }

    // 3. Process pending migrations
    for (auto it = pending_migrations_.begin(); it != pending_migrations_.end();) {
        metrics_.migrations_attempted.fetch_add(1, std::memory_order_relaxed);

        SchedulingDecision dec{};
        dec.object_id = it->object_id;
        dec.preferred_tier = it->target_tier;
        dec.allocation_size_bytes = it->size_bytes;
        dec.priority_score = it->priority;

        const auto plan_res = placement_.enqueue_request(dec);
        if (plan_res.ok()) {
            metrics_.migrations_completed.fetch_add(1, std::memory_order_relaxed);
            metrics_.total_migration_time_us.fetch_add(static_cast<uint64_t>(it->cost_estimate_us),
                                                       std::memory_order_relaxed);
            metrics_.total_bandwidth_used_bytes.fetch_add(it->size_bytes,
                                                          std::memory_order_relaxed);

            // Update local residency tracking
            const uint64_t raw_id = to_raw(it->object_id);
            auto res_it = residency_map_.find(raw_id);
            if (res_it != residency_map_.end()) {
                res_it->second.current_tier = it->target_tier;
            }

            it = pending_migrations_.erase(it);
        } else {
            metrics_.migrations_cancelled.fetch_add(1, std::memory_order_relaxed);
            it++;
        }
    }
}

Result<ResidencyInfo> MemoryOptimizer::get_residency(ObjectId object_id) const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = residency_map_.find(to_raw(object_id));
    if (it != residency_map_.end()) {
        return it->second;
    }
    return Status::kNotFound;
}

void MemoryOptimizer::set_decay_rate(double decay_rate) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    decay_rate_ = decay_rate;
}

double MemoryOptimizer::estimate_migration_cost(TierClass src, TierClass dst,
                                                uint64_t size) const noexcept {
    // Latency & bandwidth estimation constants (us / byte)
    double bw_us_per_byte = 0.0001; // default estimate
    if (src == TierClass::kVram && dst == TierClass::kRam) {
        bw_us_per_byte = 0.00005; // 20 GB/s PCIe VRAM -> RAM
    } else if (src == TierClass::kRam && dst == TierClass::kVram) {
        bw_us_per_byte = 0.00005; // 20 GB/s PCIe RAM -> VRAM
    } else if (src == TierClass::kRam && dst == TierClass::kNvme) {
        bw_us_per_byte = 0.0004; // 2.5 GB/s NVMe RAM -> SSD
    }
    return static_cast<double>(size) * bw_us_per_byte + 10.0; // bandwidth cost + base latency
}

void MemoryOptimizer::worker_loop() noexcept {
    while (is_running_.load(std::memory_order_relaxed)) {
        optimize_pass();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

} // namespace ume::event
