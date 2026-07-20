/// @file test_pattern_learning.cpp
/// @brief Google Test suite for PatternLearningEngine (Milestone M6 Task T012).

#include "ume/event/pattern_learning.h"

#include <filesystem>
#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(PatternLearningTest, IncrementalOnlineLearning) {
    PatternLearningEngine engine;

    ObjectId obj{100};
    engine.learn_access(obj, 0x1000);
    engine.learn_access(obj, 0x2000);

    auto pattern_res = engine.get_pattern(obj);
    ASSERT_TRUE(pattern_res.ok());
    EXPECT_EQ(pattern_res.value().access_count, 2U);
    EXPECT_EQ(pattern_res.value().last_address, 0x2000U);
    EXPECT_GT(pattern_res.value().frequency_score, 0.0);
}

TEST(PatternLearningTest, StridePatternConfidence) {
    PatternLearningEngine engine;

    ObjectId obj{200};
    // Access with fixed stride 4096 (1 page step)
    engine.learn_access(obj, 0x1000);
    engine.learn_access(obj, 0x2000);
    engine.learn_access(obj, 0x3000); // 2nd stride match increases confidence

    auto pattern_res = engine.get_pattern(obj);
    ASSERT_TRUE(pattern_res.ok());
    EXPECT_GT(pattern_res.value().confidence, 0.5);
    EXPECT_EQ(pattern_res.value().last_stride, 4096);
}

TEST(PatternLearningTest, SaveAndRestorePersistence) {
    PatternLearningEngine engine;
    ObjectId obj{500};
    engine.learn_access(obj, 0x5000);

    std::string path = (std::filesystem::temp_directory_path() / "ume_patterns.db").string();
    ASSERT_TRUE(engine.persist(path).ok());

    PatternLearningEngine restored_engine;
    ASSERT_TRUE(restored_engine.restore(path).ok());

    auto pattern_res = restored_engine.get_pattern(obj);
    ASSERT_TRUE(pattern_res.ok());
    EXPECT_EQ(pattern_res.value().access_count, 1U);
    EXPECT_EQ(pattern_res.value().last_address, 0x5000U);

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

} // namespace
} // namespace ume::event
