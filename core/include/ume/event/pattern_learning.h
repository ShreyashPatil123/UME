#pragma once

/// @file pattern_learning.h
/// @brief Online Pattern Learning Engine, PatternDatabase, and policies.
///
/// Part of UME Milestone M6 (Task T012).

#include "ume/status.h"
#include "ume/types.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ume::event {

/// @brief Represents a learned memory behavior profile of a specific object.
struct MemoryPattern {
    ObjectId object_id{ObjectId::kNull};
    double frequency_score{0.0};
    double recency_score{0.0};
    int64_t last_stride{0};
    double confidence{0.5};
    uint64_t last_address{0};
    uint32_t access_count{0};
    Timestamp last_seen{Timestamp::kZero};
};

/// @brief Pluggable online incremental learning policy interface.
class ILearningPolicy {
public:
    virtual ~ILearningPolicy() = default;

    /// @brief Incremental update logic for an existing pattern record.
    virtual void learn_step(MemoryPattern& pattern, uint64_t access_address,
                            Timestamp now) noexcept = 0;
};

/// @brief Default exponential moving average learning policy.
class DefaultLearningPolicy : public ILearningPolicy {
public:
    void learn_step(MemoryPattern& pattern, uint64_t access_address,
                    Timestamp now) noexcept override;
};

/// @brief Persistent storage interface for learned patterns.
class IPatternRepository {
public:
    virtual ~IPatternRepository() = default;

    virtual Result<void> save_patterns(const std::vector<MemoryPattern>& patterns,
                                       const std::string& filepath) noexcept = 0;
    virtual Result<std::vector<MemoryPattern>> load_patterns(
        const std::string& filepath) noexcept = 0;
};

/// @brief File-based pattern repository implementation.
class FilePatternRepository : public IPatternRepository {
public:
    Result<void> save_patterns(const std::vector<MemoryPattern>& patterns,
                               const std::string& filepath) noexcept override;
    Result<std::vector<MemoryPattern>> load_patterns(const std::string& filepath) noexcept override;
};

/// @brief Central learning engine receiving memory accesses and maintaining patterns.
class PatternLearningEngine {
public:
    struct Metrics {
        std::atomic<uint64_t> learned_patterns_count{0};
        std::atomic<uint64_t> active_patterns_count{0};
        std::atomic<uint64_t> forgotten_patterns_count{0};
        std::atomic<uint64_t> learning_passes{0};
        std::atomic<uint64_t> total_learning_latency_ns{0};
    };

    PatternLearningEngine(ILearningPolicy* policy = nullptr,
                          IPatternRepository* repo = nullptr) noexcept;
    ~PatternLearningEngine() = default;

    // Disable copy
    PatternLearningEngine(const PatternLearningEngine&) = delete;
    PatternLearningEngine& operator=(const PatternLearningEngine&) = delete;

    /// @brief Registers a memory access observation to drive online learning.
    void learn_access(ObjectId object_id, uint64_t address) noexcept;

    /// @brief Obtains learned pattern for a given ObjectId.
    [[nodiscard]] Result<MemoryPattern> get_pattern(ObjectId object_id) const noexcept;

    /// @brief Exports all active patterns.
    [[nodiscard]] std::vector<MemoryPattern> active_patterns() const noexcept;

    /// @brief Persists learned patterns to disk.
    Result<void> persist(const std::string& filepath) noexcept;

    /// @brief Restores learned patterns from disk.
    Result<void> restore(const std::string& filepath) noexcept;

    /// @brief Decays confidence of inactive patterns.
    void decay_patterns(double decay_factor) noexcept;

    /// @brief Resets learned database.
    void reset() noexcept;

    /// @brief Returns metrics telemetry.
    [[nodiscard]] const Metrics& metrics() const noexcept { return metrics_; }

private:
    ILearningPolicy* policy_{nullptr};
    IPatternRepository* repo_{nullptr};
    DefaultLearningPolicy default_policy_;
    FilePatternRepository default_repo_;

    mutable std::mutex mutex_;
    std::unordered_map<uint64_t, MemoryPattern> pattern_db_;
    Metrics metrics_{};
};

} // namespace ume::event
