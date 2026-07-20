/// @file test_prediction_engine.cpp
/// @brief Google Test suite for PredictionEngine (Milestone M6 Task T013).

#include "ume/event/prediction_engine.h"

#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(PredictionEngineTest, HybridPredictorChoice) {
    PatternLearningEngine learning;
    PredictionEngine engine(nullptr, &learning);

    ObjectId obj{10};
    learning.learn_access(obj, 0x1000);
    learning.learn_access(obj, 0x2000);
    learning.learn_access(obj, 0x3000);
    learning.learn_access(obj, 0x4000);
    learning.learn_access(obj, 0x5000); // warm stride, confidence increases to 0.8

    auto pred_res = engine.get_prediction(obj);
    ASSERT_TRUE(pred_res.ok());
    EXPECT_TRUE(pred_res.value().is_valid);
    EXPECT_EQ(pred_res.value().expected_tier, TierClass::kVram);
    EXPECT_GT(pred_res.value().confidence_score, 0.5);
}

TEST(PredictionEngineTest, ValidationObservationMetrics) {
    PatternLearningEngine learning;
    PredictionEngine engine(nullptr, &learning);

    ObjectId obj{20};
    learning.learn_access(obj, 0x1000);

    // Warm cache and obtain prediction
    auto pred_res = engine.get_prediction(obj);
    ASSERT_TRUE(pred_res.ok());

    // Register correct actual tier observation
    engine.register_access_observation(obj, TierClass::kRam);

    const auto& m = engine.metrics();
    EXPECT_EQ(m.predictions_issued.load(), 1U);
    EXPECT_EQ(m.correct_predictions.load(), 1U);
}

} // namespace
} // namespace ume::event
