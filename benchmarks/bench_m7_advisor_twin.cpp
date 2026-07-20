/// @file bench_m7_advisor_twin.cpp
/// @brief Google Benchmark suite for MemoryAdvisor and DigitalTwinEngine performance evaluation.

#include "ume/event/digital_twin.h"
#include "ume/event/memory_advisor.h"

#include <benchmark/benchmark.h>

namespace ume::event {

static void BM_MemoryAdvisor_GenerateReport(benchmark::State& state) {
    StatisticsCollector stats;
    PredictionEngine predictor;
    MemoryAdvisor advisor(stats, predictor);

    // Warm up stats
    stats.record_allocation(TierClass::kRam, 4096, 100);

    for (auto _ : state) {
        auto report = advisor.generate_report();
        benchmark::DoNotOptimize(report);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MemoryAdvisor_GenerateReport)->Unit(benchmark::kMicrosecond);

static void BM_DigitalTwin_SimulateStrategies(benchmark::State& state) {
    StatisticsCollector stats;
    DigitalTwinEngine twin(stats);

    SimulationContext ctx{};
    ctx.max_ram_bytes = 100 * 1024 * 1024;
    ctx.max_vram_bytes = 20 * 1024 * 1024;

    SimulationPlan p1{1, "RAM Placement", ObjectId{1}, TierClass::kRam};
    SimulationPlan p2{2, "VRAM Promotion", ObjectId{1}, TierClass::kVram};
    std::vector<SimulationPlan> candidates = {p1, p2};

    for (auto _ : state) {
        auto res = twin.simulate_strategies(ObjectId{1}, 1024, ctx, candidates);
        benchmark::DoNotOptimize(res);
    }

    state.SetItemsProcessed(state.iterations() * 2);
}
BENCHMARK(BM_DigitalTwin_SimulateStrategies)->Unit(benchmark::kMicrosecond);

} // namespace ume::event
