#pragma once

/// @file string_interner.h
/// @brief Thread-safe arena string interner with O(1) comparison.
///
/// Converts arbitrary string_views into 32-bit InternedString handles.
/// String data is stored once in a contiguous arena. Duplicate strings return
/// identical handles. String comparisons reduce to 32-bit integer equality.

#include "ume/platform/platform.h"
#include "ume/types.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ume::concurrency {

/// @brief Thread-safe string interner arena.
class StringInterner {
public:
    /// @brief Construct StringInterner with initial arena size.
    /// @param initial_arena_capacity Initial bytes to pre-allocate for string data. Default 64KB.
    explicit StringInterner(size_t initial_arena_capacity = 64 * 1024);
    ~StringInterner() = default;

    // Non-copyable, non-movable
    StringInterner(const StringInterner&) = delete;
    StringInterner& operator=(const StringInterner&) = delete;

    /// @brief Intern a string.
    ///
    /// If the string was already interned, returns the existing handle.
    /// If string is empty, returns InternedString::kNull.
    /// Thread-safe.
    ///
    /// @param str String view to intern.
    /// @return InternedString handle.
    [[nodiscard]] InternedString intern(std::string_view str);

    /// @brief Resolve an InternedString handle back to a string_view.
    ///
    /// @param handle InternedString handle.
    /// @return String view. Valid for lifetime of StringInterner. Empty view if handle is kNull.
    [[nodiscard]] std::string_view resolve(InternedString handle) const noexcept;

    /// @brief Number of unique interned strings.
    [[nodiscard]] size_t unique_string_count() const noexcept;

    /// @brief Total bytes consumed by string data arena.
    [[nodiscard]] size_t total_arena_bytes() const noexcept;

    /// @brief Clear all interned strings and reset arena. Not thread-safe with active readers.
    void clear() noexcept;

private:
    mutable std::mutex mutex_;
    std::vector<char> arena_;
    std::unordered_map<std::string_view, InternedString> map_;
    std::vector<uint32_t> offsets_;
};

} // namespace ume::concurrency
