/// @file bench_optimizer_prefetch.cpp
/// @brief Google Benchmark suite for MemoryOptimizer and PrefetchEngine performance evaluation.

#include "ume/event/memory_optimizer.h"
#include "ume/event/prefetch_engine.h"

#include <benchmark/benchmark.h>

namespace ume::event {

static void BM_MemoryOptimizer_OptimizePass(benchmark::State& state) {
    StatisticsCollector stats;
    MemoryPlacementEngine placement(100 * 1024 * 1024, 20 * 1024 * 1024, 1000 * 1024 * 1024);
    MemoryOptimizer optimizer(stats, placement);

    // Warm up objects in residency map
    for (uint64_t i = 1; i <= 1000; ++i) {
        optimizer.record_access(ObjectId{i}, 4096, TierClass::kRam);
    }

    for (auto _ : state) {
        optimizer.optimize_pass();
        // Clear placement queue periodically to prevent capacity limit errors
        if (placement.queue_size() > 500) {
            placement.clear_queue();
        }
    }

    state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(BM_MemoryOptimizer_OptimizePass)->Unit(benchmark::kMicrosecond);

static void BM_PrefetchEngine_RecordAccess(benchmark::State& state) {
    MemoryPlacementEngine placement(100 * 1024 * 1024, 20 * 1024 * 1024, 1000 * 1024 * 1024);
    PrefetchEngine engine(placement);

    ObjectId obj{100};
    uint64_t address = 0x1000;

    for (auto _ : state) {
        engine.record_access(obj, address);
        address += 0x1000; // sequential stride to trigger evaluation paths
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_PrefetchEngine_RecordAccess)->Unit(benchmark::kNanosecond);

} // namespace ume::event
