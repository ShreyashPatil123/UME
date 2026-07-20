#pragma once

/// @file prediction_engine.h
/// @brief PredictionEngine, Predictor interfaces, and HybridPredictor implementation.
///
/// Part of UME Milestone M6 (Task T013).

#include "ume/event/pattern_learning.h"
#include "ume/status.h"
#include "ume/types.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace ume::event {

/// @brief Forecasted memory access or migration output.
struct Prediction {
    ObjectId target_object{ObjectId::kNull};
    double confidence_score{0.0}; ///< Prediction confidence (0.0 ↔ 1.0)
    Timestamp expected_timestamp{Timestamp::kZero};
    TierClass expected_tier{TierClass::kRam};
    uint64_t estimated_benefit_us{0};
    uint64_t estimated_cost_us{0};
    Timestamp expiration{Timestamp::kZero};
    bool is_valid{false};
};

/// @brief Pluggable predictor algorithm interface.
class IPredictor {
public:
    virtual ~IPredictor() = default;

    /// @brief Generates prediction based on historical patterns.
    virtual Result<Prediction> predict(ObjectId object_id,
                                       const PatternLearningEngine& learning) noexcept = 0;
};

/// @brief Multi-factor predictor combining stride sequential stride and popularity weightings.
class HybridPredictor : public IPredictor {
public:
    Result<Prediction> predict(ObjectId object_id,
                               const PatternLearningEngine& learning) noexcept override;
};

/// @brief Forecasting engine issuing predictions and measuring accuracy.
class PredictionEngine {
public:
    struct Metrics {
        std::atomic<uint64_t> predictions_issued{0};
        std::atomic<uint64_t> correct_predictions{0};
        std::atomic<uint64_t> false_positives{0};
        std::atomic<uint64_t> false_negatives{0};
        std::atomic<uint64_t> cache_hits{0};
        std::atomic<uint64_t> cache_misses{0};
        std::atomic<uint64_t> total_prediction_latency_ns{0};
    };

    PredictionEngine(IPredictor* predictor = nullptr,
                     const PatternLearningEngine* learning = nullptr) noexcept;
    ~PredictionEngine() = default;

    // Disable copy
    PredictionEngine(const PredictionEngine&) = delete;
    PredictionEngine& operator=(const PredictionEngine&) = delete;

    /// @brief Requests prediction for a target object. Retrieves cached prediction if fresh.
    Result<Prediction> get_prediction(ObjectId object_id) noexcept;

    /// @brief Registers a validation observation to test accuracy of active predictions.
    void register_access_observation(ObjectId object_id, TierClass actual_tier) noexcept;

    /// @brief Invalidates expired predictions in the cache.
    void invalidate_expired_predictions() noexcept;

    /// @brief Clears prediction history.
    void reset() noexcept;

    /// @brief Returns metrics telemetry.
    [[nodiscard]] const Metrics& metrics() const noexcept { return metrics_; }

private:
    IPredictor* predictor_{nullptr};
    const PatternLearningEngine* learning_{nullptr};
    HybridPredictor default_predictor_;

    mutable std::mutex mutex_;
    std::unordered_map<uint64_t, Prediction> prediction_cache_;
    Metrics metrics_{};
};

} // namespace ume::event
