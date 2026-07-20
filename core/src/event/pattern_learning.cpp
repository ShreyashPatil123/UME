/// @file pattern_learning.cpp
/// @brief Implementation of PatternLearningEngine, policies, and file repository.

#include "ume/event/pattern_learning.h"

#include "ume/platform/clock.h"

#include <cmath>
#include <fstream>
#include <sstream>

namespace ume::event {

// ── DefaultLearningPolicy ──

void DefaultLearningPolicy::learn_step(MemoryPattern& pattern, uint64_t access_address,
                                       Timestamp now) noexcept {
    pattern.access_count++;
    pattern.last_seen = now;

    // Sequential stride calculation
    if (pattern.last_address != 0) {
        const int64_t stride =
            static_cast<int64_t>(access_address) - static_cast<int64_t>(pattern.last_address);
        if (stride == pattern.last_stride) {
            pattern.confidence = (std::min)(1.0, pattern.confidence + 0.1);
        } else {
            pattern.confidence = (std::max)(0.0, pattern.confidence - 0.05);
            pattern.last_stride = stride;
        }
    }
    pattern.last_address = access_address;

    // Moving average update for frequency and recency
    pattern.frequency_score = (pattern.frequency_score * 0.9) + 0.1;
    pattern.recency_score = 1.0; // Reset to hot recency on access
}

// ── FilePatternRepository ──

Result<void> FilePatternRepository::save_patterns(const std::vector<MemoryPattern>& patterns,
                                                  const std::string& filepath) noexcept {
    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return Status::kFileNotFound;
    }

    for (const auto& p : patterns) {
        file << to_raw(p.object_id) << "," << p.frequency_score << "," << p.recency_score << ","
             << p.last_stride << "," << p.confidence << "," << p.last_address << ","
             << p.access_count << "\n";
    }

    return Status::kOk;
}

Result<std::vector<MemoryPattern>> FilePatternRepository::load_patterns(
    const std::string& filepath) noexcept {
    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) {
        return Status::kFileNotFound;
    }

    std::vector<MemoryPattern> loaded;
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string part;
        MemoryPattern p{};

        if (!std::getline(ss, part, ','))
            continue;
        p.object_id = static_cast<ObjectId>(std::stoull(part));

        if (!std::getline(ss, part, ','))
            continue;
        p.frequency_score = std::stod(part);

        if (!std::getline(ss, part, ','))
            continue;
        p.recency_score = std::stod(part);

        if (!std::getline(ss, part, ','))
            continue;
        p.last_stride = std::stoll(part);

        if (!std::getline(ss, part, ','))
            continue;
        p.confidence = std::stod(part);

        if (!std::getline(ss, part, ','))
            continue;
        p.last_address = std::stoull(part);

        if (!std::getline(ss, part, ','))
            continue;
        p.access_count = static_cast<uint32_t>(std::stoul(part));

        loaded.push_back(p);
    }

    return loaded;
}

// ── PatternLearningEngine ──

PatternLearningEngine::PatternLearningEngine(ILearningPolicy* policy,
                                             IPatternRepository* repo) noexcept
    : policy_(policy), repo_(repo) {
    if (policy_ == nullptr)
        policy_ = &default_policy_;
    if (repo_ == nullptr)
        repo_ = &default_repo_;
}

void PatternLearningEngine::learn_access(ObjectId object_id, uint64_t address) noexcept {
    const auto start_time = platform::monotonic_now_ns();
    std::lock_guard<std::mutex> lock(mutex_);

    const uint64_t raw_id = to_raw(object_id);
    auto& pattern = pattern_db_[raw_id];
    if (pattern.object_id == ObjectId::kNull) {
        pattern.object_id = object_id;
        metrics_.learned_patterns_count.fetch_add(1, std::memory_order_relaxed);
    }

    policy_->learn_step(pattern, address, start_time);

    metrics_.learning_passes.fetch_add(1, std::memory_order_relaxed);
    metrics_.active_patterns_count.store(pattern_db_.size(), std::memory_order_relaxed);

    const auto duration = to_raw(platform::monotonic_now_ns()) - to_raw(start_time);
    metrics_.total_learning_latency_ns.fetch_add(duration, std::memory_order_relaxed);
}

Result<MemoryPattern> PatternLearningEngine::get_pattern(ObjectId object_id) const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = pattern_db_.find(to_raw(object_id));
    if (it != pattern_db_.end()) {
        return it->second;
    }
    return Status::kNotFound;
}

std::vector<MemoryPattern> PatternLearningEngine::active_patterns() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MemoryPattern> active;
    active.reserve(pattern_db_.size());
    for (const auto& kv : pattern_db_) {
        active.push_back(kv.second);
    }
    return active;
}

Result<void> PatternLearningEngine::persist(const std::string& filepath) noexcept {
    return repo_->save_patterns(active_patterns(), filepath);
}

Result<void> PatternLearningEngine::restore(const std::string& filepath) noexcept {
    const auto load_res = repo_->load_patterns(filepath);
    if (!load_res.ok()) {
        return load_res.status();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& p : load_res.value()) {
        pattern_db_[to_raw(p.object_id)] = p;
    }

    metrics_.learned_patterns_count.store(pattern_db_.size(), std::memory_order_relaxed);
    metrics_.active_patterns_count.store(pattern_db_.size(), std::memory_order_relaxed);
    return Status::kOk;
}

void PatternLearningEngine::decay_patterns(double decay_factor) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = pattern_db_.begin(); it != pattern_db_.end();) {
        it->second.confidence = (std::max)(0.0, it->second.confidence - decay_factor);
        it->second.recency_score = (std::max)(0.0, it->second.recency_score - decay_factor);

        if (it->second.confidence < 0.1 && it->second.recency_score < 0.1) {
            metrics_.forgotten_patterns_count.fetch_add(1, std::memory_order_relaxed);
            it = pattern_db_.erase(it);
        } else {
            it++;
        }
    }
    metrics_.active_patterns_count.store(pattern_db_.size(), std::memory_order_relaxed);
}

void PatternLearningEngine::reset() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    pattern_db_.clear();
    metrics_.learned_patterns_count.store(0);
    metrics_.active_patterns_count.store(0);
    metrics_.forgotten_patterns_count.store(0);
    metrics_.learning_passes.store(0);
    metrics_.total_learning_latency_ns.store(0);
}

} // namespace ume::event
