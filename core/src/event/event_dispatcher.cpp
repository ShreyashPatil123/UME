/// @file event_dispatcher.cpp
/// @brief Implementation of EventDispatcher and Subscription System.

#include "ume/event/event_dispatcher.h"

#include <algorithm>

namespace ume::event {

SubscriptionToken EventDispatcher::subscribe(IEventListener* listener,
                                             EventFilter filter) noexcept {
    if (listener == nullptr) {
        return SubscriptionToken::kInvalid;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto token = static_cast<SubscriptionToken>(++token_counter_);
    subscribers_.push_back(Subscriber{token, listener, nullptr, filter});
    return token;
}

SubscriptionToken EventDispatcher::subscribe(EventCallback callback, EventFilter filter) noexcept {
    if (!callback) {
        return SubscriptionToken::kInvalid;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto token = static_cast<SubscriptionToken>(++token_counter_);
    subscribers_.push_back(Subscriber{token, nullptr, std::move(callback), filter});
    return token;
}

Result<void> EventDispatcher::unsubscribe(SubscriptionToken token) noexcept {
    if (token == SubscriptionToken::kInvalid) {
        return Status::kInvalidArgument;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto it = std::remove_if(subscribers_.begin(), subscribers_.end(),
                                   [token](const Subscriber& sub) { return sub.token == token; });

    if (it == subscribers_.end()) {
        return Status::kNotFound;
    }

    subscribers_.erase(it, subscribers_.end());
    return Status::kOk;
}

void EventDispatcher::dispatch(const Event& event) noexcept {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (const auto& sub : subscribers_) {
        if (sub.filter.matches(event)) {
            if (sub.listener != nullptr) {
                sub.listener->on_event(event);
            } else if (sub.callback) {
                sub.callback(event);
            }
        }
    }
}

size_t EventDispatcher::subscriber_count() const noexcept {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return subscribers_.size();
}

void EventDispatcher::clear() noexcept {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    subscribers_.clear();
}

} // namespace ume::event
