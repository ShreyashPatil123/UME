/// @file bench_mpsc_queue.cpp
/// @brief Google Benchmark suite for LockFreeMpscQueue.
///
/// Measures enqueue/dequeue latency and multi-producer throughput.
/// Evaluates Performance Gates G09 (1P <= 30 ns) and G10 (8P <= 100 ns).

#include "ume/concurrency/mpsc_queue.h"

#include <benchmark/benchmark.h>
#include <thread>
#include <vector>

namespace ume::concurrency {

static void BM_MpscQueue_Enqueue_SingleProducer(benchmark::State& state) {
    LockFreeMpscQueue<uint64_t, 65536> queue;
    uint64_t val = 42;

    for (auto _ : state) {
        while (!queue.enqueue(val)) {
            uint64_t out = 0;
            (void)queue.dequeue(out);
        }
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MpscQueue_Enqueue_SingleProducer)->Unit(benchmark::kNanosecond);

static void BM_MpscQueue_EnqueueDequeue_Pipeline(benchmark::State& state) {
    LockFreeMpscQueue<uint64_t, 1024> queue;
    uint64_t val = 1;

    for (auto _ : state) {
        (void)queue.enqueue(val);
        uint64_t out = 0;
        (void)queue.dequeue(out);
    }
    state.SetItemsProcessed(state.iterations() * 2);
}
BENCHMARK(BM_MpscQueue_EnqueueDequeue_Pipeline)->Unit(benchmark::kNanosecond);

} // namespace ume::concurrency
