/// @file epoch_rcu.cpp
/// @brief Implementation of Epoch-Based Reclamation (EBR).

#include "ume/concurrency/epoch_rcu.h"

#include <algorithm>
#include <mutex>

namespace ume::concurrency {

namespace {
thread_local size_t t_thread_slot = std::numeric_limits<size_t>::max();
std::mutex g_reclaim_mutex;
} // namespace

EpochRcu::EpochRcu(uint32_t timeout_ms) noexcept : timeout_ms_(timeout_ms) {}

EpochRcu::~EpochRcu() {
    // Force reclaim all remaining retired items on destruction
    reclaim();
    for (auto& item : retired_list_) {
        if (item.deleter) {
            item.deleter();
        }
    }
    retired_list_.clear();
}

size_t EpochRcu::register_thread() noexcept {
    if (t_thread_slot != std::numeric_limits<size_t>::max()) {
        return t_thread_slot;
    }

    size_t slot = allocated_slots_.fetch_add(1, std::memory_order_relaxed);
    if (slot >= kMaxRcuThreads) {
        // Fallback to slot 0 if slots exhausted
        slot = 0;
    }

    t_thread_slot = slot;
    return slot;
}

void EpochRcu::unregister_thread(size_t slot_index) noexcept {
    if (slot_index < kMaxRcuThreads) {
        slots_[slot_index].active.store(false, std::memory_order_release);
        slots_[slot_index].epoch.store(0, std::memory_order_release);
    }
    if (t_thread_slot == slot_index) {
        t_thread_slot = std::numeric_limits<size_t>::max();
    }
}

void EpochRcu::enter(size_t slot_index) noexcept {
    if (slot_index >= kMaxRcuThreads)
        return;

    auto& slot = slots_[slot_index];
    const uint64_t current_g_epoch = global_epoch_.load(std::memory_order_relaxed);

    slot.entry_time_ns.store(platform::to_ns(platform::monotonic_now_ns()),
                             std::memory_order_relaxed);
    slot.epoch.store(current_g_epoch, std::memory_order_relaxed);
    slot.active.store(true, std::memory_order_release);
}

void EpochRcu::exit(size_t slot_index) noexcept {
    if (slot_index >= kMaxRcuThreads)
        return;

    auto& slot = slots_[slot_index];
    slot.active.store(false, std::memory_order_release);
}

void EpochRcu::retire(std::function<void()> deleter) {
    if (!deleter)
        return;

    const uint64_t current_g_epoch = global_epoch_.load(std::memory_order_relaxed);

    std::lock_guard<std::mutex> lock(g_reclaim_mutex);
    retired_list_.push_back(
        DeferredDeletion{.epoch = current_g_epoch, .deleter = std::move(deleter)});
}

bool EpochRcu::try_advance_epoch() noexcept {
    const uint64_t current_g_epoch = global_epoch_.load(std::memory_order_relaxed);
    const size_t num_slots =
        std::min(allocated_slots_.load(std::memory_order_relaxed), kMaxRcuThreads);

    // Check if any active thread is still on an older epoch
    for (size_t i = 0; i < num_slots; ++i) {
        const auto& slot = slots_[i];
        if (slot.active.load(std::memory_order_acquire)) {
            const uint64_t thread_epoch = slot.epoch.load(std::memory_order_relaxed);
            if (thread_epoch < current_g_epoch) {
                // Check for stalled reader timeout
                const uint64_t now_ns = platform::to_ns(platform::monotonic_now_ns());
                const uint64_t entry_ns = slot.entry_time_ns.load(std::memory_order_relaxed);
                const uint64_t elapsed_ms =
                    (now_ns > entry_ns) ? (now_ns - entry_ns) / 1'000'000ULL : 0;

                if (elapsed_ms < timeout_ms_) {
                    return false; // Still active and not timed out
                }
                // Timed out reader — force advance
            }
        }
    }

    // All active threads are at or above current_g_epoch
    global_epoch_.fetch_add(1, std::memory_order_release);
    return true;
}

size_t EpochRcu::reclaim() noexcept {
    try_advance_epoch();

    const uint64_t min_active_epoch = [this]() -> uint64_t {
        uint64_t min_epoch = global_epoch_.load(std::memory_order_relaxed);
        const size_t num_slots =
            std::min(allocated_slots_.load(std::memory_order_relaxed), kMaxRcuThreads);
        for (size_t i = 0; i < num_slots; ++i) {
            if (slots_[i].active.load(std::memory_order_acquire)) {
                min_epoch = std::min(min_epoch, slots_[i].epoch.load(std::memory_order_relaxed));
            }
        }
        return min_epoch;
    }();

    std::vector<DeferredDeletion> to_delete;
    {
        std::lock_guard<std::mutex> lock(g_reclaim_mutex);
        auto it = std::remove_if(retired_list_.begin(), retired_list_.end(),
                                 [min_active_epoch, &to_delete](const DeferredDeletion& item) {
                                     if (item.epoch < min_active_epoch) {
                                         to_delete.push_back(item);
                                         return true;
                                     }
                                     return false;
                                 });
        retired_list_.erase(it, retired_list_.end());
    }

    size_t count = to_delete.size();
    for (auto& item : to_delete) {
        if (item.deleter) {
            item.deleter();
        }
    }
    return count;
}

size_t EpochRcu::pending_reclamation_count() const noexcept {
    std::lock_guard<std::mutex> lock(g_reclaim_mutex);
    return retired_list_.size();
}

} // namespace ume::concurrency
