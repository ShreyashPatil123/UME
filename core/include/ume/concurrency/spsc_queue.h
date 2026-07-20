#pragma once

/// @file spsc_queue.h
/// @brief Lock-free Single-Producer Single-Consumer (SPSC) queue.

#include <atomic>
#include <cstddef>
#include <type_traits>
#include <utility>

#ifndef UME_CACHE_LINE_SIZE
    #define UME_CACHE_LINE_SIZE 64
#endif

namespace ume::concurrency {

/// @brief A bounded single-producer, single-consumer (SPSC) lock-free queue.
/// @tparam T The type of elements held in the queue.
/// @tparam Capacity The maximum number of elements the queue can hold (must be a power of two).
template <typename T, std::size_t Capacity>
class LockFreeSpscQueue {
    static_assert(Capacity > 0, "Capacity must be greater than 0");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two");

public:
    LockFreeSpscQueue() noexcept : head_(0), cached_tail_(0), tail_(0), cached_head_(0) {}

    ~LockFreeSpscQueue() = default;

    LockFreeSpscQueue(const LockFreeSpscQueue&) = delete;
    LockFreeSpscQueue& operator=(const LockFreeSpscQueue&) = delete;

    bool push(const T& item) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        std::size_t head = head_.load(std::memory_order_relaxed);

        if (head - cached_tail_ >= Capacity) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (head - cached_tail_ >= Capacity) {
                return false; // Queue is full
            }
        }

        buffer_[head & kMask] = item;
        head_.store(head + 1, std::memory_order_release);
        return true;
    }

    bool push(T&& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::size_t head = head_.load(std::memory_order_relaxed);

        if (head - cached_tail_ >= Capacity) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (head - cached_tail_ >= Capacity) {
                return false; // Queue is full
            }
        }

        buffer_[head & kMask] = std::move(item);
        head_.store(head + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& out_item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::size_t tail = tail_.load(std::memory_order_relaxed);

        if (cached_head_ == tail) {
            cached_head_ = head_.load(std::memory_order_acquire);
            if (cached_head_ == tail) {
                return false; // Queue is empty
            }
        }

        out_item = std::move(buffer_[tail & kMask]);
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    const T* front() const noexcept {
        std::size_t tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return nullptr; // Queue is empty
        }

        return &buffer_[tail & kMask];
    }

    std::size_t capacity() const noexcept { return Capacity; }

    bool empty() const noexcept { return approx_size() == 0; }

    std::size_t approx_size() const noexcept {
        std::size_t h = head_.load(std::memory_order_acquire);
        std::size_t t = tail_.load(std::memory_order_acquire);
        return (h >= t) ? (h - t) : 0;
    }

private:
    static constexpr std::size_t kMask = Capacity - 1;

    alignas(UME_CACHE_LINE_SIZE) std::atomic<std::size_t> head_;
    alignas(UME_CACHE_LINE_SIZE) std::size_t cached_tail_;

    alignas(UME_CACHE_LINE_SIZE) std::atomic<std::size_t> tail_;
    alignas(UME_CACHE_LINE_SIZE) std::size_t cached_head_;

    T buffer_[Capacity];
};

} // namespace ume::concurrency
