/// @file bench_scheduler_placement.cpp
/// @brief Google Benchmark suite for MemoryScheduler and MemoryPlacementEngine performance.

#include "ume/event/memory_placement.h"
#include "ume/event/memory_scheduler.h"

#include <benchmark/benchmark.h>

namespace ume::event {

static void BM_MemoryScheduler_ScheduleAllocation(benchmark::State& state) {
    StatisticsCollector stats;
    EventAnalyzer analyzer(&stats);
    MemoryScheduler scheduler(stats, analyzer);

    for (auto _ : state) {
        auto decision = scheduler.schedule_allocation(ObjectId{1}, 4096, 5);
        benchmark::DoNotOptimize(decision);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MemoryScheduler_ScheduleAllocation)->Unit(benchmark::kNanosecond);

static void BM_MemoryPlacementEngine_PlanAndQueue(benchmark::State& state) {
    MemoryPlacementEngine engine(100 * 1024 * 1024, 20 * 1024 * 1024, 1000 * 1024 * 1024);

    SchedulingDecision decision{};
    decision.object_id = ObjectId{1};
    decision.allocation_size_bytes = 4096;
    decision.preferred_tier = TierClass::kRam;

    for (auto _ : state) {
        auto plan_res = engine.plan_placement(decision);
        benchmark::DoNotOptimize(plan_res);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MemoryPlacementEngine_PlanAndQueue)->Unit(benchmark::kNanosecond);

} // namespace ume::event
