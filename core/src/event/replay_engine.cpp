/// @file replay_engine.cpp
/// @brief Implementation of ReplayEngine reading journal segments and delivering events to
/// subscribers.

#include "ume/event/replay_engine.h"

namespace ume::event {

Result<ReplayStats> ReplayEngine::replay_segment(const std::string& segment_path,
                                                 EventDispatcher& dispatcher) noexcept {
    JournalReader reader;
    const auto open_res = reader.open_segment(segment_path);
    if (!open_res.ok()) {
        return open_res.status();
    }

    ReplayStats stats{};
    Event ev{};

    while (reader.read_next(ev).ok()) {
        ev.header.flags = ev.header.flags | EventFlags::kReplayed;
        dispatcher.dispatch(ev);
        stats.events_replayed++;
    }

    const auto& reader_stats = reader.stats();
    stats.segments_replayed = reader_stats.segments_read;
    stats.corrupted_records_skipped = reader_stats.corrupted_records_skipped;
    stats.bytes_read = reader_stats.bytes_read;

    return stats;
}

Result<ReplayStats> ReplayEngine::replay_directory(const std::string& journal_directory,
                                                   EventDispatcher& dispatcher) noexcept {
    JournalReader reader;
    const auto open_res = reader.open_directory(journal_directory);
    if (!open_res.ok()) {
        return open_res.status();
    }

    ReplayStats stats{};
    Event ev{};

    while (reader.read_next(ev).ok()) {
        ev.header.flags = ev.header.flags | EventFlags::kReplayed;
        dispatcher.dispatch(ev);
        stats.events_replayed++;
    }

    const auto& reader_stats = reader.stats();
    stats.segments_replayed = reader_stats.segments_read;
    stats.corrupted_records_skipped = reader_stats.corrupted_records_skipped;
    stats.bytes_read = reader_stats.bytes_read;

    return stats;
}

} // namespace ume::event
