#pragma once

/// @file engine_provider.h
/// @brief Provider interfaces decoupling QML ViewModels from the UME engine backends.

#include "ume/event/digital_twin.h"
#include "ume/event/event_analyzer.h"
#include "ume/event/memory_advisor.h"
#include "ume/event/memory_statistics.h"
#include "ume/event/prediction_engine.h"
#include "ume/status.h"

#include <string>
#include <vector>

namespace ume::dashboard {

/// @brief Telemetry provider interface for memory statistics.
class IStatisticsProvider {
public:
    virtual ~IStatisticsProvider() = default;
    virtual event::MemoryStatisticsSnapshot get_statistics() const noexcept = 0;
};

/// @brief Telemetry provider interface for prediction engine.
class IPredictionProvider {
public:
    virtual ~IPredictionProvider() = default;
    virtual double get_accuracy() const noexcept = 0;
};

/// @brief Telemetry provider interface for memory advisor.
class IAdvisorProvider {
public:
    virtual ~IAdvisorProvider() = default;
    virtual event::AdvisorReport get_advisor_report() noexcept = 0;
};

/// @brief Telemetry provider interface for digital twin.
class IDigitalTwinProvider {
public:
    virtual ~IDigitalTwinProvider() = default;
    virtual std::vector<event::SimulationPlan> get_evaluated_plans() const noexcept = 0;
};

} // namespace ume::dashboard
