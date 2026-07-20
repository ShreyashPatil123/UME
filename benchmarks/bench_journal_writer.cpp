/// @file bench_journal_writer.cpp
/// @brief Google Benchmark suite for JournalWriter high-throughput append engine.

#include "ume/event/journal_writer.h"

#include <benchmark/benchmark.h>
#include <filesystem>

namespace ume::event {

static void BM_JournalWriter_SingleThreadAppend(benchmark::State& state) {
    const std::string test_dir =
        (std::filesystem::temp_directory_path() / "ume_bench_journal").string();
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    JournalWriter writer;
    const auto open_res = writer.open(test_dir, 1024ULL * 1024ULL * 1024ULL); // 1 GB segment
    if (!open_res.ok()) {
        state.SkipWithError("Failed to open journal writer");
        return;
    }

    Event ev = Event::create_object_event(
        EventId{1}, Timestamp{1000ULL}, EventType::kObjectCreated, ObjectId{10}, TierId{1}, 4096);

    for (auto _ : state) {
        auto res = writer.append(ev);
        benchmark::DoNotOptimize(res);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * (sizeof(JournalRecordHeader) + sizeof(Event)));

    (void)writer.close();
    std::filesystem::remove_all(test_dir, ec);
}
BENCHMARK(BM_JournalWriter_SingleThreadAppend)->Unit(benchmark::kNanosecond);

} // namespace ume::event
