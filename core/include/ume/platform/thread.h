#pragma once

/// @file thread.h
/// @brief Cross-platform thread management utilities.
///
/// Provides thread naming, CPU core pinning (affinity), thread priority,
/// and hardware concurrency query functions.

#include "ume/status.h"
#include "ume/types.h"

#include <cstddef>
#include <cstdint>

namespace ume::platform {

/// @brief Thread priority levels.
enum class ThreadPriority : uint8_t {
    kIdle = 0,
    kLow = 1,
    kNormal = 2,
    kHigh = 3,
    kRealtime = 4,
};

/// @brief Set descriptive name for the current calling thread (visible in debuggers/profilers).
///
/// @param name Null-terminated thread name (truncated to 15 chars on Linux).
/// @return kOk on success, kInvalidArgument if name is null.
Status set_current_thread_name(const char* name) noexcept;

/// @brief Pin current calling thread to a specific logical CPU core.
///
/// @param cpu_core Core index (0 to get_hardware_concurrency() - 1).
/// @return kOk on success, kInvalidArgument if core index out of range.
Status set_current_thread_affinity(size_t cpu_core) noexcept;

/// @brief Set execution priority for current calling thread.
///
/// @param priority Priority level.
/// @return kOk on success, error status otherwise.
Status set_current_thread_priority(ThreadPriority priority) noexcept;

/// @brief Get total number of logical CPU cores available on system.
[[nodiscard]] size_t get_hardware_concurrency() noexcept;

} // namespace ume::platform
