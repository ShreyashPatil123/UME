#pragma once

/// @file real_engine_adapter.h
/// @brief Adapter implementing engine provider interfaces using real UME backend engines.

#include "engine_provider.h"
#include "ume/event/event_analyzer.h"
#include "ume/event/event_dispatcher.h"
#include "ume/event/pattern_learning.h"

#include <memory>
#include <mutex>
#include <QObject>
#include <QTimer>

namespace ume::dashboard {

class RealEngineAdapter : public QObject,
                          public IStatisticsProvider,
                          public IPredictionProvider,
                          public IAdvisorProvider,
                          public IDigitalTwinProvider {
    Q_OBJECT
public:
    explicit RealEngineAdapter(QObject* parent = nullptr) noexcept;
    ~RealEngineAdapter() override = default;

    // Interface implementations
    event::MemoryStatisticsSnapshot get_statistics() const noexcept override;
    double get_accuracy() const noexcept override;
    event::AdvisorReport get_advisor_report() noexcept override;
    std::vector<event::SimulationPlan> get_evaluated_plans() const noexcept override;

    // Real components references
    event::StatisticsCollector& stats() noexcept { return stats_; }
    event::PredictionEngine& predictor() noexcept { return predictor_; }
    event::MemoryAdvisor& advisor() noexcept { return advisor_; }
    event::DigitalTwinEngine& twin() noexcept { return twin_; }

signals:
    void data_changed();

private slots:
    void process_simulation_tick() noexcept;

private:
    QTimer timer_;
    mutable std::mutex mutex_;

    // Real UME backend engines
    event::StatisticsCollector stats_;
    event::EventAnalyzer analyzer_;
    event::EventDispatcher dispatcher_;
    event::PatternLearningEngine learning_;
    event::PredictionEngine predictor_;
    event::MemoryAdvisor advisor_;
    event::DigitalTwinEngine twin_;

    // Simulated/Real cache states
    std::vector<event::SimulationPlan> last_evaluated_plans_;
    double prediction_accuracy_{0.94};
    uint64_t ticks_count_{0};
};

} // namespace ume::dashboard
