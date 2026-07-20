/// @file bench_replay_dispatcher.cpp
/// @brief Google Benchmark suite for EventDispatcher throughput and Replay + Dispatch Pipeline
/// latency.

#include "ume/event/event_dispatcher.h"
#include "ume/event/journal_writer.h"
#include "ume/event/replay_engine.h"

#include <benchmark/benchmark.h>
#include <filesystem>

namespace ume::event {

static void BM_EventDispatcher_DispatchSingleListener(benchmark::State& state) {
    EventDispatcher dispatcher;
    uint64_t counter = 0;
    dispatcher.subscribe([&counter](const Event&) { counter++; });

    Event ev = Event::create_object_event(EventId{1}, Timestamp{1000ULL}, EventType::kObjectCreated,
                                          ObjectId{10}, TierId{1}, 4096);

    for (auto _ : state) {
        dispatcher.dispatch(ev);
        benchmark::DoNotOptimize(counter);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_EventDispatcher_DispatchSingleListener)->Unit(benchmark::kNanosecond);

static void BM_ReplayEngine_ReplayDirectory(benchmark::State& state) {
    const std::string test_dir =
        (std::filesystem::temp_directory_path() / "ume_bench_replay").string();
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    JournalWriter writer;
    (void)writer.open(test_dir, 64 * 1024 * 1024);

    constexpr size_t kEventsInJournal = 10000;
    for (size_t i = 0; i < kEventsInJournal; ++i) {
        Event ev =
            Event::create_object_event(EventId{i + 1}, Timestamp{1000ULL},
                                       EventType::kObjectCreated, ObjectId{10}, TierId{1}, 4096);
        (void)writer.append(ev);
    }
    (void)writer.close();

    EventDispatcher dispatcher;
    uint64_t count = 0;
    dispatcher.subscribe([&count](const Event&) { count++; });

    ReplayEngine replay;

    for (auto _ : state) {
        count = 0;
        auto stats_res = replay.replay_directory(test_dir, dispatcher);
        benchmark::DoNotOptimize(stats_res);
    }

    state.SetItemsProcessed(state.iterations() * kEventsInJournal);
    state.SetBytesProcessed(state.iterations() * kEventsInJournal *
                            (sizeof(JournalRecordHeader) + sizeof(Event)));

    std::filesystem::remove_all(test_dir, ec);
}
BENCHMARK(BM_ReplayEngine_ReplayDirectory)->Unit(benchmark::kMillisecond);

} // namespace ume::event
