#pragma once

/// @file event_dispatcher.h
/// @brief High-throughput in-memory Event Dispatcher and Subscription System.
///
/// Part of UME Milestone M2 (Task T005).

#include "ume/event/event.h"
#include "ume/status.h"
#include "ume/types.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace ume::event {

/// @brief Strongly typed subscription token for unsubscribing.
enum class SubscriptionToken : uint64_t {
    kInvalid = 0,
};

/// @brief Interface for object-based event listeners.
class IEventListener {
public:
    virtual ~IEventListener() = default;

    /// @brief Callback invoked when a matching event is dispatched.
    virtual void on_event(const Event& event) noexcept = 0;
};

/// @brief Filter criteria for event subscription.
struct EventFilter {
    EventCategory category{EventCategory::kUnknown}; ///< kUnknown = match all categories
    EventType type{EventType::kUnknown};             ///< kUnknown = match all types
    EventPriority min_priority{EventPriority::kLow}; ///< Match events >= min_priority

    /// @brief Returns true if an event satisfies the filter criteria.
    [[nodiscard]] constexpr bool matches(const Event& event) const noexcept {
        if (category != EventCategory::kUnknown && event.header.category != category) {
            return false;
        }
        if (type != EventType::kUnknown && event.header.type != type) {
            return false;
        }
        if (static_cast<uint8_t>(event.header.priority) < static_cast<uint8_t>(min_priority)) {
            return false;
        }
        return true;
    }
};

/// @brief In-memory Event Dispatcher supporting listeners, callbacks, and filtering.
class EventDispatcher {
public:
    using EventCallback = std::function<void(const Event&)>;

    EventDispatcher() noexcept = default;
    ~EventDispatcher() = default;

    // Disable copy
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;

    /// @brief Subscribes an object-based listener with optional filter criteria.
    SubscriptionToken subscribe(IEventListener* listener, EventFilter filter = {}) noexcept;

    /// @brief Subscribes a functional callback with optional filter criteria.
    SubscriptionToken subscribe(EventCallback callback, EventFilter filter = {}) noexcept;

    /// @brief Unsubscribes a listener or callback using its token.
    Result<void> unsubscribe(SubscriptionToken token) noexcept;

    /// @brief Dispatches an event to all matching subscribers.
    ///
    /// Thread-safe and concurrency optimized using shared read locks for 0 lock contention.
    void dispatch(const Event& event) noexcept;

    /// @brief Returns total active subscriber count.
    [[nodiscard]] size_t subscriber_count() const noexcept;

    /// @brief Removes all active subscriptions.
    void clear() noexcept;

private:
    struct Subscriber {
        SubscriptionToken token{SubscriptionToken::kInvalid};
        IEventListener* listener{nullptr};
        EventCallback callback{nullptr};
        EventFilter filter{};
    };

    mutable std::shared_mutex mutex_;
    std::vector<Subscriber> subscribers_;
    uint64_t token_counter_{0};
};

} // namespace ume::event
