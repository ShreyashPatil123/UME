/// @file bench_epoch_rcu.cpp
/// @brief Google Benchmark suite for Epoch RCU.
///
/// Measures read-side EpochGuard enter/exit latency.
/// Evaluates Performance Gate G08 (RCU read <= 15 ns).

#include "ume/concurrency/epoch_rcu.h"

#include <benchmark/benchmark.h>

namespace ume::concurrency {

static void BM_EpochRcu_ReadSideGuard(benchmark::State& state) {
    EpochRcu rcu(100);
    size_t slot = rcu.register_thread();

    for (auto _ : state) {
        rcu.enter(slot);
        benchmark::DoNotOptimize(rcu.current_epoch());
        rcu.exit(slot);
    }

    rcu.unregister_thread(slot);
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_EpochRcu_ReadSideGuard)->Unit(benchmark::kNanosecond);

static void BM_EpochRcu_RAVI_EpochGuard(benchmark::State& state) {
    EpochRcu rcu(100);

    for (auto _ : state) {
        EpochGuard guard(rcu);
        benchmark::DoNotOptimize(rcu.current_epoch());
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_EpochRcu_RAVI_EpochGuard)->Unit(benchmark::kNanosecond);

} // namespace ume::concurrency
