/// @file test_config.cpp
/// @brief Unit tests for UME configuration parsing and validation.

#include "ume/config.h"

#include <gtest/gtest.h>

namespace ume {
namespace {

TEST(ConfigTest, DefaultTestConfig_JournalSegmentSize) {
    Config config = Config::default_test_config();
    EXPECT_EQ(config.journal.segment_size_bytes, 1024ULL * 1024ULL);
}

TEST(ConfigTest, DefaultTestConfig_SmallShardCount) {
    Config config = Config::default_test_config();
    EXPECT_EQ(config.object_store.shard_count, 16U);
}

TEST(ConfigTest, DefaultTestConfig_ProbesDisabled) {
    Config config = Config::default_test_config();
    EXPECT_FALSE(config.probes.cuda_enabled);
    EXPECT_FALSE(config.probes.nvml_enabled);
}

TEST(ConfigTest, DefaultTestConfig_LogLevelDebug) {
    Config config = Config::default_test_config();
    EXPECT_EQ(config.engine.log_level, "debug");
}

TEST(ConfigTest, DefaultTestConfig_AdvisorDisabled) {
    Config config = Config::default_test_config();
    EXPECT_FALSE(config.advisor.enabled);
}

TEST(ConfigTest, Validate_DefaultTestConfig_IsValid) {
    Config config = Config::default_test_config();
    EXPECT_TRUE(is_ok(config.validate()));
}

TEST(ConfigTest, Validate_DefaultConfig_IsValid) {
    Config config{};  // Default-constructed config should be valid
    EXPECT_TRUE(is_ok(config.validate()));
}

TEST(ConfigTest, Validate_ShardCountNotPowerOfTwo_ReturnsInvalidConfig) {
    Config config = Config::default_test_config();
    config.object_store.shard_count = 15;
    EXPECT_EQ(config.validate(), Status::kInvalidConfig);
}

TEST(ConfigTest, Validate_ShardCountZero_ReturnsInvalidConfig) {
    Config config = Config::default_test_config();
    config.object_store.shard_count = 0;
    EXPECT_EQ(config.validate(), Status::kInvalidConfig);
}

TEST(ConfigTest, Validate_SegmentSizeTooSmall_ReturnsInvalidConfig) {
    Config config = Config::default_test_config();
    config.journal.segment_size_bytes = 1024 * 1024 - 1;  // Just under 1 MB
    EXPECT_EQ(config.validate(), Status::kInvalidConfig);
}

TEST(ConfigTest, Validate_SegmentSizeExactlyOneMB_IsValid) {
    Config config = Config::default_test_config();
    config.journal.segment_size_bytes = 1024 * 1024;  // Exactly 1 MB
    EXPECT_TRUE(is_ok(config.validate()));
}

TEST(ConfigTest, Validate_CpuBudgetNegative_ReturnsInvalidConfig) {
    Config config = Config::default_test_config();
    config.engine.overhead_budget_cpu_percent = -1.0f;
    EXPECT_EQ(config.validate(), Status::kInvalidConfig);
}

TEST(ConfigTest, Validate_CpuBudgetOverHundred_ReturnsInvalidConfig) {
    Config config = Config::default_test_config();
    config.engine.overhead_budget_cpu_percent = 101.0f;
    EXPECT_EQ(config.validate(), Status::kInvalidConfig);
}

TEST(ConfigTest, Validate_MemoryBudgetTooSmall_ReturnsInvalidConfig) {
    Config config = Config::default_test_config();
    config.engine.overhead_budget_memory_mb = 9;
    EXPECT_EQ(config.validate(), Status::kInvalidConfig);
}

TEST(ConfigTest, Validate_SampleRateZero_ReturnsInvalidConfig) {
    Config config = Config::default_test_config();
    config.probes.sample_rate_hz = 0;
    EXPECT_EQ(config.validate(), Status::kInvalidConfig);
}

TEST(ConfigTest, Validate_SampleRateTooHigh_ReturnsInvalidConfig) {
    Config config = Config::default_test_config();
    config.probes.sample_rate_hz = 1001;
    EXPECT_EQ(config.validate(), Status::kInvalidConfig);
}

TEST(ConfigTest, LoadFromFile_NonexistentFile_ReturnsFileNotFound) {
    auto result = Config::load_from_file("/nonexistent/path/to/file.toml");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.status(), Status::kFileNotFound);
}

} // namespace
} // namespace ume
