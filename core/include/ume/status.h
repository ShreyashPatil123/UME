#pragma once

/// @file status.h
/// @brief Status codes and Result type for fallible operations.
///
/// UME does not use C++ exceptions. All fallible operations return either
/// a Status code or a Result<T> which wraps a value and a status code.
/// This is part of the frozen v3 architecture specification.

#include <cassert>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ume {

// ──────────────────────────────────────────────────────────────────────
// Status Codes
// ──────────────────────────────────────────────────────────────────────

/// @brief Status codes for UME operations.
///
/// Status codes are grouped by category:
/// - 0x00xx: Success
/// - 0x01xx: Invalid argument / precondition violations
/// - 0x02xx: Resource errors (out of memory, capacity)
/// - 0x03xx: State errors (not found, already exists)
/// - 0x04xx: I/O errors (journal, storage)
/// - 0x05xx: Concurrency errors
/// - 0x06xx: Plugin errors
/// - 0xFFxx: Internal errors
enum class Status : uint16_t {
    // ── Success ──
    kOk = 0x0000,

    // ── Invalid Argument ──
    kInvalidArgument = 0x0100, ///< Generic invalid argument
    kNullPointer = 0x0101,     ///< Unexpected null pointer
    kOutOfRange = 0x0102,      ///< Value outside valid range
    kInvalidState = 0x0103,    ///< Invalid state transition
    kInvalidConfig = 0x0104,   ///< Invalid configuration value
    kInvalidIr = 0x0105,       ///< Invalid UM-IR instruction sequence

    // ── Resource Errors ──
    kOutOfMemory = 0x0200,      ///< Memory allocation failed
    kCapacityExceeded = 0x0201, ///< Queue/buffer/store capacity exceeded
    kBudgetExceeded = 0x0202,   ///< Performance budget exceeded

    // ── State Errors ──
    kNotFound = 0x0300,           ///< Object/tier/edge not found
    kAlreadyExists = 0x0301,      ///< Object/tier/edge already exists
    kNotInitialized = 0x0302,     ///< Engine/module not initialized
    kAlreadyInitialized = 0x0303, ///< Engine/module already initialized
    kShutdown = 0x0304,           ///< Engine is shutting down

    // ── I/O Errors ──
    kIoError = 0x0400,            ///< Generic I/O error
    kJournalCorrupted = 0x0401,   ///< Journal segment CRC mismatch
    kJournalFull = 0x0402,        ///< Journal retention limit reached
    kSerializationError = 0x0403, ///< Serialization/deserialization failed
    kFileNotFound = 0x0404,       ///< File does not exist
    kPermissionDenied = 0x0405,   ///< Insufficient permissions

    // ── Concurrency Errors ──
    kTimeout = 0x0500,      ///< Operation timed out
    kContention = 0x0501,   ///< Lock contention (retry advised)
    kEpochStalled = 0x0502, ///< RCU epoch advancement stalled

    // ── Plugin Errors ──
    kPluginLoadFailed = 0x0600,      ///< Failed to load plugin library
    kPluginVersionMismatch = 0x0601, ///< Plugin ABI version mismatch
    kPluginInitFailed = 0x0602,      ///< Plugin initialization failed
    kPluginNotFound = 0x0603,        ///< Plugin not registered

    // ── Internal Errors ──
    kInternalError = 0xFF00, ///< Generic internal error (bug)
    kUnimplemented = 0xFF01, ///< Feature not yet implemented
};

/// @brief Returns true if the status represents a successful operation.
[[nodiscard]] constexpr bool is_ok(Status s) noexcept {
    return s == Status::kOk;
}

/// @brief Returns true if the status represents an error.
[[nodiscard]] constexpr bool is_error(Status s) noexcept {
    return s != Status::kOk;
}

/// @brief Returns a human-readable name for a status code.
///
/// @param s The status code.
/// @return A string_view with the status name (e.g., "kOk", "kNotFound").
///         The returned view points to static storage and is always valid.
[[nodiscard]] constexpr std::string_view status_name(Status s) noexcept {
    switch (s) {
        case Status::kOk:
            return "kOk";
        case Status::kInvalidArgument:
            return "kInvalidArgument";
        case Status::kNullPointer:
            return "kNullPointer";
        case Status::kOutOfRange:
            return "kOutOfRange";
        case Status::kInvalidState:
            return "kInvalidState";
        case Status::kInvalidConfig:
            return "kInvalidConfig";
        case Status::kInvalidIr:
            return "kInvalidIr";
        case Status::kOutOfMemory:
            return "kOutOfMemory";
        case Status::kCapacityExceeded:
            return "kCapacityExceeded";
        case Status::kBudgetExceeded:
            return "kBudgetExceeded";
        case Status::kNotFound:
            return "kNotFound";
        case Status::kAlreadyExists:
            return "kAlreadyExists";
        case Status::kNotInitialized:
            return "kNotInitialized";
        case Status::kAlreadyInitialized:
            return "kAlreadyInitialized";
        case Status::kShutdown:
            return "kShutdown";
        case Status::kIoError:
            return "kIoError";
        case Status::kJournalCorrupted:
            return "kJournalCorrupted";
        case Status::kJournalFull:
            return "kJournalFull";
        case Status::kSerializationError:
            return "kSerializationError";
        case Status::kFileNotFound:
            return "kFileNotFound";
        case Status::kPermissionDenied:
            return "kPermissionDenied";
        case Status::kTimeout:
            return "kTimeout";
        case Status::kContention:
            return "kContention";
        case Status::kEpochStalled:
            return "kEpochStalled";
        case Status::kPluginLoadFailed:
            return "kPluginLoadFailed";
        case Status::kPluginVersionMismatch:
            return "kPluginVersionMismatch";
        case Status::kPluginInitFailed:
            return "kPluginInitFailed";
        case Status::kPluginNotFound:
            return "kPluginNotFound";
        case Status::kInternalError:
            return "kInternalError";
        case Status::kUnimplemented:
            return "kUnimplemented";
    }
    return "kUnknown";
}

// ──────────────────────────────────────────────────────────────────────
// Result<T>
// ──────────────────────────────────────────────────────────────────────

/// @brief A result type that holds either a value or an error status.
///
/// Result<T> is the primary mechanism for returning fallible values in UME.
/// It is inspired by Rust's Result<T, E> and absl::StatusOr<T>.
///
/// Usage:
/// @code
///     Result<const MemoryObject*> result = store.find(id);
///     if (result.ok()) {
///         const MemoryObject* obj = result.value();
///     } else {
///         log_error("Failed: {}", status_name(result.status()));
///     }
/// @endcode
///
/// @tparam T The value type. Must be movable. Can be a pointer type.
template <typename T>
class [[nodiscard]] Result {
public:
    /// @brief Construct a successful result with a value.
    /// @param value The value to wrap.
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(T value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value_(std::move(value)), status_(Status::kOk) {}

    /// @brief Construct an error result with a status code.
    /// @param status The error status. Must not be kOk.
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(Status status) noexcept : status_(status) {
        assert(status != Status::kOk && "Cannot construct error Result with kOk status");
    }

    /// @brief Returns true if the result contains a value.
    [[nodiscard]] bool ok() const noexcept { return is_ok(status_); }

    /// @brief Returns the status code.
    [[nodiscard]] Status status() const noexcept { return status_; }

    /// @brief Returns the contained value.
    /// @pre ok() must be true. Undefined behavior otherwise.
    [[nodiscard]] T& value() & noexcept {
        assert(ok() && "Accessing value of error Result");
        return value_;
    }

    /// @brief Returns the contained value (const).
    /// @pre ok() must be true. Undefined behavior otherwise.
    [[nodiscard]] const T& value() const& noexcept {
        assert(ok() && "Accessing value of error Result");
        return value_;
    }

    /// @brief Returns the contained value (rvalue).
    /// @pre ok() must be true. Undefined behavior otherwise.
    [[nodiscard]] T&& value() && noexcept {
        assert(ok() && "Accessing value of error Result");
        return std::move(value_);
    }

    /// @brief Returns the value if ok, or a default value if error.
    /// @param default_value The fallback value.
    /// @return The contained value or the default.
    [[nodiscard]] T value_or(T default_value) const& noexcept(
        std::is_nothrow_move_constructible_v<T>) {
        return ok() ? value_ : std::move(default_value);
    }

    /// @brief Implicit conversion to bool. Returns true if ok().
    [[nodiscard]] explicit operator bool() const noexcept { return ok(); }

private:
    T value_{};
    Status status_;
};

/// @brief Specialization for void results (status-only, no value).
///
/// Used for operations that can fail but don't return a value.
///
/// Usage:
/// @code
///     Result<void> result = engine.start();
///     if (!result.ok()) {
///         log_error("Engine start failed: {}", status_name(result.status()));
///     }
/// @endcode
template <>
class [[nodiscard]] Result<void> {
public:
    /// @brief Construct a successful void result.
    Result() noexcept : status_(Status::kOk) {}

    /// @brief Construct a result from a status code.
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(Status status) noexcept : status_(status) {}

    /// @brief Returns true if the result is successful.
    [[nodiscard]] bool ok() const noexcept { return is_ok(status_); }

    /// @brief Returns the status code.
    [[nodiscard]] Status status() const noexcept { return status_; }

    /// @brief Implicit conversion to bool. Returns true if ok().
    [[nodiscard]] explicit operator bool() const noexcept { return ok(); }

private:
    Status status_;
};

// ──────────────────────────────────────────────────────────────────────
// Macros
// ──────────────────────────────────────────────────────────────────────

/// @brief Return early if a status is an error.
///
/// Usage:
/// @code
///     UME_RETURN_IF_ERROR(journal.append(event));
///     // Only reached if append() returned kOk.
/// @endcode
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UME_RETURN_IF_ERROR(expr)                     \
    do {                                              \
        auto ume_status_macro_tmp_ = (expr);          \
        if (::ume::is_error(ume_status_macro_tmp_)) { \
            return ume_status_macro_tmp_;             \
        }                                             \
    } while (false)

/// @brief Assign a Result's value or return early if it's an error.
///
/// Usage:
/// @code
///     UME_ASSIGN_OR_RETURN(auto obj, store.find(id));
///     // obj is now the unwrapped value.
/// @endcode
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UME_ASSIGN_OR_RETURN(lhs, expr)                               \
    auto UME_CONCAT_(ume_result_macro_tmp_, __LINE__) = (expr);       \
    if (!UME_CONCAT_(ume_result_macro_tmp_, __LINE__).ok()) {         \
        return UME_CONCAT_(ume_result_macro_tmp_, __LINE__).status(); \
    }                                                                 \
    lhs = std::move(UME_CONCAT_(ume_result_macro_tmp_, __LINE__)).value()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UME_CONCAT_(a, b) UME_CONCAT_INNER_(a, b)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UME_CONCAT_INNER_(a, b) a##b

} // namespace ume
