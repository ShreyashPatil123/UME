#pragma once

/// @file hazard_pointers.h
/// @brief Hazard Pointers memory reclamation for lock-free reader threads.
///
/// Hazard Pointers allow reader threads to publish a pointer currently being
/// dereferenced. Writer threads scan all active hazard pointers before freeing
/// retired nodes, guaranteeing no use-after-free errors.

#include "ume/platform/platform.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace ume::concurrency {

/// @brief Maximum number of hazard pointer slots supported.
constexpr size_t kMaxHazardSlots = 128;

/// @brief Single hazard pointer slot.
struct alignas(UME_CACHE_LINE_SIZE) HazardSlot {
    std::atomic<const void*> pointer{nullptr};
    std::atomic<bool> active{false};
};

/// @brief Hazard Pointer Domain manager.
class HazardPointerDomain {
public:
    HazardPointerDomain() noexcept = default;
    ~HazardPointerDomain() {
        reclaim_all();
    }

    // Non-copyable, non-movable
    HazardPointerDomain(const HazardPointerDomain&) = delete;
    HazardPointerDomain& operator=(const HazardPointerDomain&) = delete;

    /// @brief Acquire a hazard pointer slot for current reader thread.
    [[nodiscard]] size_t acquire_slot() noexcept {
        for (size_t i = 0; i < kMaxHazardSlots; ++i) {
            bool expected = false;
            if (slots_[i].active.compare_exchange_strong(expected, true, std::memory_order_relaxed)) {
                return i;
            }
        }
        return kMaxHazardSlots; // Exhausted
    }

    /// @brief Release a hazard pointer slot.
    void release_slot(size_t slot_index) noexcept {
        if (slot_index < kMaxHazardSlots) {
            slots_[slot_index].pointer.store(nullptr, std::memory_order_release);
            slots_[slot_index].active.store(false, std::memory_order_release);
        }
    }

    /// @brief Protect a pointer in a slot.
    void protect(size_t slot_index, const void* ptr) noexcept {
        if (slot_index < kMaxHazardSlots) {
            slots_[slot_index].pointer.store(ptr, std::memory_order_release);
        }
    }

    /// @brief Clear protection in a slot.
    void clear(size_t slot_index) noexcept {
        if (slot_index < kMaxHazardSlots) {
            slots_[slot_index].pointer.store(nullptr, std::memory_order_release);
        }
    }

    /// @brief Retire a pointer for deferred deletion.
    /// @param ptr Pointer to node.
    /// @param deleter Function to free node memory.
    void retire(const void* ptr, std::function<void(const void*)> deleter) {
        if (!ptr || !deleter) return;
        retired_.push_back({ptr, std::move(deleter)});
        reclaim();
    }

    /// @brief Reclaim all retired pointers not protected by any active hazard pointer.
    size_t reclaim() noexcept {
        if (retired_.empty()) return 0;

        // Collect all currently published hazard pointers
        std::vector<const void*> protected_ptrs;
        protected_ptrs.reserve(kMaxHazardSlots);
        for (size_t i = 0; i < kMaxHazardSlots; ++i) {
            if (slots_[i].active.load(std::memory_order_relaxed)) {
                const void* p = slots_[i].pointer.load(std::memory_order_acquire);
                if (p) {
                    protected_ptrs.push_back(p);
                }
            }
        }

        std::vector<RetiredItem> still_retired;
        size_t reclaimed_count = 0;

        for (auto& item : retired_) {
            bool is_protected = false;
            for (const void* p : protected_ptrs) {
                if (p == item.ptr) {
                    is_protected = true;
                    break;
                }
            }

            if (is_protected) {
                still_retired.push_back(std::move(item));
            } else {
                if (item.deleter) {
                    item.deleter(item.ptr);
                }
                reclaimed_count++;
            }
        }

        retired_ = std::move(still_retired);
        return reclaimed_count;
    }

    /// @brief Force reclaim all retired pointers unconditionally.
    void reclaim_all() noexcept {
        for (auto& item : retired_) {
            if (item.deleter) {
                item.deleter(item.ptr);
            }
        }
        retired_.clear();
    }

private:
    struct RetiredItem {
        const void* ptr;
        std::function<void(const void*)> deleter;
    };

    alignas(UME_CACHE_LINE_SIZE) std::array<HazardSlot, kMaxHazardSlots> slots_{};
    std::vector<RetiredItem> retired_;
};

} // namespace ume::concurrency
