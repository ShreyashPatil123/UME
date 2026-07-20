/// @file string_interner.cpp
/// @brief Implementation of thread-safe arena string interner.

#include "ume/concurrency/string_interner.h"

#include <cstring>

namespace ume::concurrency {

StringInterner::StringInterner(size_t initial_arena_capacity) {
    arena_.reserve(initial_arena_capacity);
    offsets_.reserve(1024);
    // Index 0 reserved for empty string / kNull
    offsets_.push_back(0);
}

InternedString StringInterner::intern(std::string_view str) {
    if (str.empty()) {
        return InternedString::kNull;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // Check if string is already interned
    auto it = map_.find(str);
    if (it != map_.end()) {
        return it->second;
    }

    // Append string to arena (including null terminator)
    const uint32_t offset = static_cast<uint32_t>(arena_.size());
    const size_t len = str.length();

    arena_.insert(arena_.end(), str.begin(), str.end());
    arena_.push_back('\0');

    // Create string_view pointing directly to the arena memory
    std::string_view arena_view(arena_.data() + offset, len);

    const auto handle_val = static_cast<uint32_t>(offsets_.size());
    offsets_.push_back(offset);

    const auto handle = static_cast<InternedString>(handle_val);
    map_[arena_view] = handle;

    return handle;
}

std::string_view StringInterner::resolve(InternedString handle) const noexcept {
    const auto handle_val = static_cast<uint32_t>(handle);
    if (handle_val == 0) {
        return std::string_view{};
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (handle_val >= offsets_.size()) {
        return std::string_view{};
    }

    const uint32_t offset = offsets_[handle_val];
    const char* str_ptr = arena_.data() + offset;
    return std::string_view(str_ptr, std::strlen(str_ptr));
}

size_t StringInterner::unique_string_count() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return map_.size();
}

size_t StringInterner::total_arena_bytes() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return arena_.size();
}

void StringInterner::clear() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    arena_.clear();
    map_.clear();
    offsets_.clear();
    offsets_.push_back(0);
}

} // namespace ume::concurrency
