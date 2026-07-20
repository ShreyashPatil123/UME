/// @file bench_event_analysis.cpp
/// @brief Google Benchmark suite for EventAnalyzer throughput and StatisticsCollector update
/// latency.

#include "ume/event/event_analyzer.h"
#include "ume/event/memory_statistics.h"

#include <benchmark/benchmark.h>

namespace ume::event {

static void BM_EventAnalyzer_ProcessEvent(benchmark::State& state) {
    StatisticsCollector stats;
    EventAnalyzer analyzer(&stats);

    Event alloc_ev = Event::create_object_event(
        EventId{1}, Timestamp{1000ULL}, EventType::kObjectCreated, ObjectId{10}, TierId{1}, 4096);
    Event free_ev = Event::create_object_event(
        EventId{2}, Timestamp{2000ULL}, EventType::kObjectDestroyed, ObjectId{10}, TierId{1}, 4096);

    for (auto _ : state) {
        analyzer.process_event(alloc_ev);
        analyzer.process_event(free_ev);
    }

    state.SetItemsProcessed(state.iterations() * 2);
}
BENCHMARK(BM_EventAnalyzer_ProcessEvent)->Unit(benchmark::kNanosecond);

static void BM_StatisticsCollector_RecordAllocation(benchmark::State& state) {
    StatisticsCollector stats;

    for (auto _ : state) {
        stats.record_allocation(TierClass::kRam, 4096, 100);
        stats.record_free(TierClass::kRam, 4096, 5000, 100);
    }

    state.SetItemsProcessed(state.iterations() * 2);
}
BENCHMARK(BM_StatisticsCollector_RecordAllocation)->Unit(benchmark::kNanosecond);

} // namespace ume::event
