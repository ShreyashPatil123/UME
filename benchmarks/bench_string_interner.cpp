/// @file bench_string_interner.cpp
/// @brief Google Benchmark suite for StringInterner.
///
/// Measures string interning latency and handle resolution performance.

#include "ume/concurrency/string_interner.h"

#include <benchmark/benchmark.h>

namespace ume::concurrency {

static void BM_StringInterner_InternNewString(benchmark::State& state) {
    StringInterner interner;
    uint64_t counter = 0;

    for (auto _ : state) {
        std::string s = "tensor_object_name_" + std::to_string(counter++);
        auto h = interner.intern(s);
        benchmark::DoNotOptimize(h);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_StringInterner_InternNewString)->Unit(benchmark::kNanosecond);

static void BM_StringInterner_LookupExistingString(benchmark::State& state) {
    StringInterner interner;
    constexpr std::string_view kTarget = "vram_tier_0_allocation_block";
    auto initial_handle = interner.intern(kTarget);
    benchmark::DoNotOptimize(initial_handle);

    for (auto _ : state) {
        auto h = interner.intern(kTarget);
        benchmark::DoNotOptimize(h);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_StringInterner_LookupExistingString)->Unit(benchmark::kNanosecond);

static void BM_StringInterner_ResolveHandle(benchmark::State& state) {
    StringInterner interner;
    auto h = interner.intern("kv_cache_layer_31_head_7");

    for (auto _ : state) {
        auto view = interner.resolve(h);
        benchmark::DoNotOptimize(view);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_StringInterner_ResolveHandle)->Unit(benchmark::kNanosecond);

} // namespace ume::concurrency
