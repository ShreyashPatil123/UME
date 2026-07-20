/// @file prediction_engine.cpp
/// @brief Implementation of PredictionEngine and HybridPredictor forecasting algorithms.

#include "ume/event/prediction_engine.h"

#include "ume/platform/clock.h"

namespace ume::event {

// ── HybridPredictor ──

Result<Prediction> HybridPredictor::predict(ObjectId object_id,
                                            const PatternLearningEngine& learning) noexcept {
    const auto pattern_res = learning.get_pattern(object_id);
    if (!pattern_res.ok()) {
        return pattern_res.status();
    }

    const MemoryPattern p = pattern_res.value();

    Prediction pred{};
    pred.target_object = object_id;
    pred.confidence_score = p.confidence;
    pred.is_valid = true;

    // Stride-based sequential stride forecast
    if (p.last_stride != 0 && p.confidence > 0.7) {
        pred.expected_tier = TierClass::kVram; // promote hot sequential patterns
        pred.estimated_benefit_us = 150;
        pred.estimated_cost_us = 15;
    } else if (p.frequency_score > 0.8) {
        pred.expected_tier = TierClass::kVram; // promote highly popular objects
        pred.estimated_benefit_us = 100;
        pred.estimated_cost_us = 20;
    } else if (p.recency_score < 0.2) {
        pred.expected_tier = TierClass::kNvme; // demote cold objects to SSD
        pred.estimated_benefit_us = 50;
        pred.estimated_cost_us = 80;
    } else {
        pred.expected_tier = TierClass::kRam; // standard memory
        pred.estimated_benefit_us = 0;
        pred.estimated_cost_us = 0;
    }

    // Set expiration 5 seconds in future
    pred.expected_timestamp =
        static_cast<Timestamp>(to_raw(platform::monotonic_now_ns()) + 1000000ULL);
    pred.expiration = static_cast<Timestamp>(to_raw(platform::monotonic_now_ns()) + 5000000000ULL);

    return pred;
}

// ── PredictionEngine ──

PredictionEngine::PredictionEngine(IPredictor* predictor,
                                   const PatternLearningEngine* learning) noexcept
    : predictor_(predictor), learning_(learning) {
    if (predictor_ == nullptr)
        predictor_ = &default_predictor_;
}

Result<Prediction> PredictionEngine::get_prediction(ObjectId object_id) noexcept {
    const auto start_time = platform::monotonic_now_ns();
    std::lock_guard<std::mutex> lock(mutex_);

    const uint64_t raw_id = to_raw(object_id);

    // Check cache
    const auto it = prediction_cache_.find(raw_id);
    if (it != prediction_cache_.end()) {
        const auto now = platform::monotonic_now_ns();
        if (to_raw(it->second.expiration) > 0 && to_raw(now) < to_raw(it->second.expiration)) {
            metrics_.cache_hits.fetch_add(1, std::memory_order_relaxed);
            return it->second;
        }
        // Invalidate expired item
        prediction_cache_.erase(it);
    }

    metrics_.cache_misses.fetch_add(1, std::memory_order_relaxed);

    if (learning_ == nullptr) {
        return Status::kInvalidState;
    }

    const auto pred_res = predictor_->predict(object_id, *learning_);
    if (!pred_res.ok()) {
        return pred_res.status();
    }

    prediction_cache_[raw_id] = pred_res.value();
    metrics_.predictions_issued.fetch_add(1, std::memory_order_relaxed);

    const auto duration = to_raw(platform::monotonic_now_ns()) - to_raw(start_time);
    metrics_.total_prediction_latency_ns.fetch_add(duration, std::memory_order_relaxed);

    return pred_res.value();
}

void PredictionEngine::register_access_observation(ObjectId object_id,
                                                   TierClass actual_tier) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    const uint64_t raw_id = to_raw(object_id);

    const auto it = prediction_cache_.find(raw_id);
    if (it != prediction_cache_.end()) {
        if (it->second.expected_tier == actual_tier) {
            metrics_.correct_predictions.fetch_add(1, std::memory_order_relaxed);
        } else {
            metrics_.false_positives.fetch_add(1, std::memory_order_relaxed);
        }
        // Clear validated prediction from cache
        prediction_cache_.erase(it);
    } else {
        metrics_.false_negatives.fetch_add(1, std::memory_order_relaxed);
    }
}

void PredictionEngine::invalidate_expired_predictions() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto now = platform::monotonic_now_ns();

    for (auto it = prediction_cache_.begin(); it != prediction_cache_.end();) {
        if (to_raw(it->second.expiration) > 0 && to_raw(now) >= to_raw(it->second.expiration)) {
            it = prediction_cache_.erase(it);
        } else {
            it++;
        }
    }
}

void PredictionEngine::reset() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    prediction_cache_.clear();
    metrics_.predictions_issued.store(0);
    metrics_.correct_predictions.store(0);
    metrics_.false_positives.store(0);
    metrics_.false_negatives.store(0);
    metrics_.cache_hits.store(0);
    metrics_.cache_misses.store(0);
    metrics_.total_prediction_latency_ns.store(0);
}

} // namespace ume::event
