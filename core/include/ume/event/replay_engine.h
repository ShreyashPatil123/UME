#pragma once

/// @file replay_engine.h
/// @brief Replay Engine reading journal segments and dispatching events to subscribers.
///
/// Part of UME Milestone M2 (Task T004).

#include "ume/event/event_dispatcher.h"
#include "ume/event/journal_reader.h"
#include "ume/status.h"

#include <cstdint>
#include <string>

namespace ume::event {

/// @brief Statistics output for journal replay operations.
struct ReplayStats {
    uint64_t segments_replayed{0};
    uint64_t events_replayed{0};
    uint64_t corrupted_records_skipped{0};
    uint64_t bytes_read{0};
};

/// @brief Journal Replay Engine reading binary journal segments and triggering dispatch.
class ReplayEngine {
public:
    ReplayEngine() noexcept = default;
    ~ReplayEngine() = default;

    // Disable copy
    ReplayEngine(const ReplayEngine&) = delete;
    ReplayEngine& operator=(const ReplayEngine&) = delete;

    /// @brief Replays all events from a single journal segment file (.umej) to a dispatcher.
    ///
    /// @param segment_path Path to .umej segment file.
    /// @param dispatcher Destination event dispatcher.
    /// @return Result containing ReplayStats or error status.
    Result<ReplayStats> replay_segment(const std::string& segment_path,
                                       EventDispatcher& dispatcher) noexcept;

    /// @brief Replays all events from all .umej segments in a directory to a dispatcher.
    ///
    /// @param journal_directory Directory path containing .umej files.
    /// @param dispatcher Destination event dispatcher.
    /// @return Result containing ReplayStats or error status.
    Result<ReplayStats> replay_directory(const std::string& journal_directory,
                                         EventDispatcher& dispatcher) noexcept;
};

} // namespace ume::event
