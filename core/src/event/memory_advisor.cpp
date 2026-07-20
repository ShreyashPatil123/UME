/// @file memory_advisor.cpp
/// @brief Implementation of MemoryAdvisor and default HealthAdvisorRule.

#include "ume/event/memory_advisor.h"

#include "ume/platform/clock.h"

namespace ume::event {

// ── HealthAdvisorRule ──

std::vector<Recommendation> HealthAdvisorRule::analyze(const StatisticsCollector& stats,
                                                       const PredictionEngine& predictor) noexcept {
    (void)predictor;
    std::vector<Recommendation> recs;

    const MemoryStatisticsSnapshot snap = stats.snapshot();

    // Check high VRAM pressure
    if (snap.peak_vram_bytes > 0) {
        const double vram_ratio = static_cast<double>(snap.current_vram_bytes) /
                                  static_cast<double>(snap.peak_vram_bytes);
        if (vram_ratio > 0.9) {
            Recommendation rec{};
            rec.category = "Memory Health";
            rec.severity = AdvisorSeverity::kCritical;
            rec.description = "VRAM pressure exceeds critical limit.";
            rec.root_cause = "Current active video memory demands exceed 90% of observed peaks.";
            rec.suggested_action = "Demote cold objects to RAM or enable SSD spilling fallbacks.";
            rec.estimated_benefit_us = 2500;
            rec.confidence_score = 0.95;
            rec.timestamp = platform::monotonic_now_ns();
            recs.push_back(rec);
        }
    }

    // Check RAM pressure
    if (snap.peak_ram_bytes > 0) {
        const double ram_ratio =
            static_cast<double>(snap.current_ram_bytes) / static_cast<double>(snap.peak_ram_bytes);
        if (ram_ratio > 0.85) {
            Recommendation rec{};
            rec.category = "Memory Health";
            rec.severity = AdvisorSeverity::kWarning;
            rec.description = "System DRAM utilization is high.";
            rec.root_cause = "Active host allocation demands exceed 85% of peak RAM.";
            rec.suggested_action = "Evict cold allocations to NVMe SSD storage.";
            rec.estimated_benefit_us = 1000;
            rec.confidence_score = 0.85;
            rec.timestamp = platform::monotonic_now_ns();
            recs.push_back(rec);
        }
    }

    return recs;
}

// ── MemoryAdvisor ──

MemoryAdvisor::MemoryAdvisor(const StatisticsCollector& stats,
                             const PredictionEngine& predictor) noexcept
    : stats_(stats), predictor_(predictor) {
    rules_.push_back(&default_health_rule_);
}

void MemoryAdvisor::register_rule(IAdvisorRule* rule) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    if (rule != nullptr) {
        rules_.push_back(rule);
    }
}

AdvisorReport MemoryAdvisor::generate_report() noexcept {
    const auto start_time = platform::monotonic_now_ns();
    std::lock_guard<std::mutex> lock(mutex_);

    AdvisorReport report{};
    report.timestamp = start_time;

    double cumulative_deductions = 0.0;

    for (auto* rule : rules_) {
        auto recs = rule->analyze(stats_, predictor_);
        for (const auto& rec : recs) {
            report.recommendations.push_back(rec);

            // Deduct system health score based on severity
            switch (rec.severity) {
                case AdvisorSeverity::kCritical:
                    cumulative_deductions += 25.0;
                    break;
                case AdvisorSeverity::kHigh:
                    cumulative_deductions += 15.0;
                    break;
                case AdvisorSeverity::kWarning:
                    cumulative_deductions += 5.0;
                    break;
                default:
                    break;
            }
        }
    }

    report.health_score = (std::max)(0.0, 100.0 - cumulative_deductions);

    metrics_.reports_generated.fetch_add(1, std::memory_order_relaxed);

    const auto duration = to_raw(platform::monotonic_now_ns()) - to_raw(start_time);
    metrics_.total_analysis_time_ns.fetch_add(duration, std::memory_order_relaxed);

    return report;
}

} // namespace ume::event
