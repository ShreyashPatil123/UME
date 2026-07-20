#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#ifndef UME_CACHE_LINE_SIZE
#define UME_CACHE_LINE_SIZE 64
#endif

namespace ume::concurrency {

/// @brief A bounded multi-producer, single-consumer (MPSC) lock-free queue based on Dmitry Vyukov's design.
/// @tparam T The type of elements held in the queue.
/// @tparam Capacity The maximum number of elements the queue can hold (must be a power of two).
template <typename T, std::size_t Capacity>
class LockFreeMpscQueue {
    static_assert(Capacity > 0, "Capacity must be greater than 0");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two");

public:
    /// @brief Constructs an empty MPSC queue.
    LockFreeMpscQueue() noexcept {
        for (std::size_t i = 0; i < Capacity; ++i) {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
        enqueue_cursor_.store(0, std::memory_order_relaxed);
        dequeue_cursor_.store(0, std::memory_order_relaxed);
    }

    /// @brief Destructor.
    ~LockFreeMpscQueue() = default;

    LockFreeMpscQueue(const LockFreeMpscQueue&) = delete;
    LockFreeMpscQueue& operator=(const LockFreeMpscQueue&) = delete;

    /// @brief Attempts to enqueue an element into the queue.
    /// @param data The element to enqueue.
    /// @return True if the element was successfully enqueued, false if the queue is full.
    bool enqueue(const T& data) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        Cell* cell = nullptr;
        std::size_t pos = enqueue_cursor_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & kMask];
            std::size_t seq = cell->sequence.load(std::memory_order_acquire);
            std::intptr_t diff = static_cast<std::intptr_t>(seq) - static_cast<std::intptr_t>(pos);

            if (diff == 0) {
                if (enqueue_cursor_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (diff < 0) {
                return false; // Queue is full
            } else {
                pos = enqueue_cursor_.load(std::memory_order_relaxed);
            }
        }

        cell->data = data;
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    /// @brief Attempts to enqueue an element into the queue (move semantics).
    /// @param data The element to enqueue.
    /// @return True if the element was successfully enqueued, false if the queue is full.
    bool enqueue(T&& data) noexcept(std::is_nothrow_move_assignable_v<T>) {
        Cell* cell = nullptr;
        std::size_t pos = enqueue_cursor_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & kMask];
            std::size_t seq = cell->sequence.load(std::memory_order_acquire);
            std::intptr_t diff = static_cast<std::intptr_t>(seq) - static_cast<std::intptr_t>(pos);

            if (diff == 0) {
                if (enqueue_cursor_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (diff < 0) {
                return false; // Queue is full
            } else {
                pos = enqueue_cursor_.load(std::memory_order_relaxed);
            }
        }

        cell->data = std::move(data);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    /// @brief Attempts to dequeue an element from the queue.
    /// @param out_data Reference to store the dequeued element.
    /// @return True if an element was successfully dequeued, false if the queue is empty.
    bool dequeue(T& out_data) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::size_t pos = dequeue_cursor_.load(std::memory_order_relaxed);
        Cell* cell = &buffer_[pos & kMask];
        std::size_t seq = cell->sequence.load(std::memory_order_acquire);
        std::intptr_t diff = static_cast<std::intptr_t>(seq) - static_cast<std::intptr_t>(pos + 1);

        if (diff == 0) {
            out_data = std::move(cell->data);
            cell->sequence.store(pos + kMask + 1, std::memory_order_release);
            dequeue_cursor_.store(pos + 1, std::memory_order_relaxed);
            return true;
        }

        return false; // Queue is empty
    }

    /// @brief Returns the maximum capacity of the queue.
    /// @return The capacity of the queue.
    std::size_t capacity() const noexcept {
        return Capacity;
    }

    /// @brief Checks if the queue is currently empty.
    /// @return True if the queue is empty, false otherwise.
    bool empty() const noexcept {
        return approx_size() == 0;
    }

    /// @brief Returns the approximate number of elements in the queue.
    /// @return The approximate size of the queue.
    std::size_t approx_size() const noexcept {
        std::size_t head = enqueue_cursor_.load(std::memory_order_acquire);
        std::size_t tail = dequeue_cursor_.load(std::memory_order_acquire);
        if (head >= tail) {
            return head - tail;
        }
        return 0;
    }

private:
    struct alignas(UME_CACHE_LINE_SIZE) Cell {
        std::atomic<std::size_t> sequence;
        T data;
    };

    static constexpr std::size_t kMask = Capacity - 1;

    alignas(UME_CACHE_LINE_SIZE) Cell buffer_[Capacity];
    alignas(UME_CACHE_LINE_SIZE) std::atomic<std::size_t> enqueue_cursor_;
    alignas(UME_CACHE_LINE_SIZE) std::atomic<std::size_t> dequeue_cursor_;
};

} // namespace ume::concurrency
