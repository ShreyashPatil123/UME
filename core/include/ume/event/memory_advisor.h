#pragma once

/// @file memory_advisor.h
/// @brief Memory Advisor generating actionable recommendations from telemetry and predictions.
///
/// Part of UME Milestone M7 (Task T014).

#include "ume/event/memory_statistics.h"
#include "ume/event/prediction_engine.h"
#include "ume/status.h"
#include "ume/types.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ume::event {

/// @brief Severity level for advisory recommendations.
enum class AdvisorSeverity : uint8_t {
    kInfo = 0,
    kWarning = 1,
    kHigh = 2,
    kCritical = 3,
};

/// @brief Human-readable actionable advice recommendation.
struct Recommendation {
    std::string category;
    AdvisorSeverity severity{AdvisorSeverity::kInfo};
    std::string description;
    std::string root_cause;
    std::string suggested_action;
    uint64_t estimated_benefit_us{0};
    double confidence_score{0.5};
    Timestamp timestamp{Timestamp::kZero};
};

/// @brief Modular rule interface for analyzing engine state and telemetry.
class IAdvisorRule {
public:
    virtual ~IAdvisorRule() = default;

    /// @brief Performs analysis and returns generated recommendations.
    virtual std::vector<Recommendation> analyze(const StatisticsCollector& stats,
                                                const PredictionEngine& predictor) noexcept = 0;
};

/// @brief Health rule monitoring memory pressure.
class HealthAdvisorRule : public IAdvisorRule {
public:
    std::vector<Recommendation> analyze(const StatisticsCollector& stats,
                                        const PredictionEngine& predictor) noexcept override;
};

/// @brief Complete advisor intelligence analysis report output.
struct AdvisorReport {
    std::vector<Recommendation> recommendations;
    double health_score{100.0}; ///< Target overall system health score (0.0 ↔ 100.0)
    Timestamp timestamp{Timestamp::kZero};
};

/// @brief Decision advice engine driving rule checks and recommendations.
class MemoryAdvisor {
public:
    struct Metrics {
        std::atomic<uint64_t> reports_generated{0};
        std::atomic<uint64_t> recommendations_accepted{0};
        std::atomic<uint64_t> total_analysis_time_ns{0};
    };

    MemoryAdvisor(const StatisticsCollector& stats, const PredictionEngine& predictor) noexcept;
    ~MemoryAdvisor() = default;

    // Disable copy
    MemoryAdvisor(const MemoryAdvisor&) = delete;
    MemoryAdvisor& operator=(const MemoryAdvisor&) = delete;

    /// @brief Registers a modular advisor rule.
    void register_rule(IAdvisorRule* rule) noexcept;

    /// @brief Triggers all rules to generate a comprehensive AdvisorReport.
    [[nodiscard]] AdvisorReport generate_report() noexcept;

    /// @brief Obtains metrics telemetry.
    [[nodiscard]] const Metrics& metrics() const noexcept { return metrics_; }

private:
    const StatisticsCollector& stats_;
    const PredictionEngine& predictor_;
    std::vector<IAdvisorRule*> rules_;
    HealthAdvisorRule default_health_rule_;

    mutable std::mutex mutex_;
    Metrics metrics_{};
};

} // namespace ume::event
