/// @file bench_m6_learning_prediction.cpp
/// @brief Google Benchmark suite for PatternLearningEngine and PredictionEngine performance
/// assessment.

#include "ume/event/pattern_learning.h"
#include "ume/event/prediction_engine.h"

#include <benchmark/benchmark.h>

namespace ume::event {

static void BM_PatternLearningEngine_LearnAccess(benchmark::State& state) {
    PatternLearningEngine learning;

    ObjectId obj{123};
    uint64_t address = 0x1000;

    for (auto _ : state) {
        learning.learn_access(obj, address);
        address += 0x1000;
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_PatternLearningEngine_LearnAccess)->Unit(benchmark::kNanosecond);

static void BM_PredictionEngine_GetPrediction(benchmark::State& state) {
    PatternLearningEngine learning;
    PredictionEngine predictor(nullptr, &learning);

    ObjectId obj{456};
    learning.learn_access(obj, 0x1000);
    learning.learn_access(obj, 0x2000);
    learning.learn_access(obj, 0x3000);

    for (auto _ : state) {
        auto pred = predictor.get_prediction(obj);
        benchmark::DoNotOptimize(pred);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_PredictionEngine_GetPrediction)->Unit(benchmark::kNanosecond);

} // namespace ume::event
