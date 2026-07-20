#pragma once

/// @file epoch_rcu.h
/// @brief Epoch-Based Reclamation (EBR) for lock-free read access.
///
/// Epoch RCU allows reader threads to read shared data structures (such as the
/// Object Table or Memory Graph) with near-zero overhead (~10 ns) while writer
/// threads safely reclaim old memory nodes after all active readers have exited
/// the epoch.
///
/// Performance Target: Gate G08 (Read-side cost <= 15 ns).

#include "ume/platform/clock.h"
#include "ume/platform/platform.h"
#include "ume/status.h"
#include "ume/types.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace ume::concurrency {

/// @brief Maximum number of reader threads tracked by the Epoch RCU system.
constexpr size_t kMaxRcuThreads = 64;

/// @brief State of a registered reader thread slot.
struct alignas(UME_CACHE_LINE_SIZE) ThreadEpochSlot {
    std::atomic<uint64_t> epoch{0};      ///< Current epoch read by thread (0 = inactive)
    std::atomic<bool> active{false};     ///< True if thread is currently reading
    std::atomic<uint64_t> entry_time_ns{0}; ///< Entry timestamp for timeout detection
    std::atomic<uint32_t> thread_id{0};  ///< OS thread ID
};

/// @brief Pending deletion record.
struct DeferredDeletion {
    uint64_t epoch;                      ///< Epoch when item was retired
    std::function<void()> deleter;      ///< Cleanup callback
};

/// @brief Epoch-Based Reclamation Manager.
///
/// Usage:
/// @code
///     EpochRcu rcu;
///     
///     // Reader thread:
///     {
///         EpochGuard guard(rcu);
///         // Read RCU-protected pointer safely
///     }
///     
///     // Writer thread:
///     rcu.retire([]() { delete old_node; });
///     rcu.synchronize(); // Or call reclaim() periodically
/// @endcode
class EpochRcu {
public:
    /// @brief Construct EpochRcu manager.
    /// @param timeout_ms Max time a reader can hold an epoch before being flagged. Default 100ms.
    explicit EpochRcu(uint32_t timeout_ms = 100) noexcept;
    ~EpochRcu();

    // Non-copyable, non-movable
    EpochRcu(const EpochRcu&) = delete;
    EpochRcu& operator=(const EpochRcu&) = delete;

    /// @brief Register current thread as a reader. Returns slot index.
    [[nodiscard]] size_t register_thread() noexcept;

    /// @brief Unregister current thread.
    void unregister_thread(size_t slot_index) noexcept;

    /// @brief Enter read side critical section.
    /// @param slot_index Thread's registered slot index.
    void enter(size_t slot_index) noexcept;

    /// @brief Exit read side critical section.
    /// @param slot_index Thread's registered slot index.
    void exit(size_t slot_index) noexcept;

    /// @brief Retire a resource for deletion in a future epoch.
    /// @param deleter Callback to free the resource.
    void retire(std::function<void()> deleter);

    /// @brief Advance global epoch if all active readers have left the minimum epoch.
    /// @return True if epoch was advanced.
    bool try_advance_epoch() noexcept;

    /// @brief Reclaim all retired objects whose epoch has passed.
    /// @return Number of objects deleted.
    size_t reclaim() noexcept;

    /// @brief Get current global epoch.
    [[nodiscard]] uint64_t current_epoch() const noexcept {
        return global_epoch_.load(std::memory_order_relaxed);
    }

    /// @brief Get count of pending retired objects waiting for reclamation.
    [[nodiscard]] size_t pending_reclamation_count() const noexcept;

private:
    alignas(UME_CACHE_LINE_SIZE) std::atomic<uint64_t> global_epoch_{1};
    alignas(UME_CACHE_LINE_SIZE) std::array<ThreadEpochSlot, kMaxRcuThreads> slots_{};
    alignas(UME_CACHE_LINE_SIZE) std::atomic<size_t> allocated_slots_{0};

    // Retired objects pending deletion
    alignas(UME_CACHE_LINE_SIZE) std::vector<DeferredDeletion> retired_list_;
    uint32_t timeout_ms_;
};

/// @brief RAII guard for Epoch RCU read critical sections.
class EpochGuard {
public:
    /// @brief Enter RCU read critical section.
    /// @param rcu Reference to EpochRcu instance.
    explicit EpochGuard(EpochRcu& rcu) noexcept
        : rcu_(rcu), slot_index_(rcu.register_thread()) {
        rcu_.enter(slot_index_);
    }

    ~EpochGuard() noexcept {
        rcu_.exit(slot_index_);
    }

    // Non-copyable, non-movable
    EpochGuard(const EpochGuard&) = delete;
    EpochGuard& operator=(const EpochGuard&) = delete;

private:
    EpochRcu& rcu_;
    size_t slot_index_;
};

} // namespace ume::concurrency
