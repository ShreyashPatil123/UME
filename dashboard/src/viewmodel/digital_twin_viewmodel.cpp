/// @file digital_twin_viewmodel.cpp
/// @brief DigitalTwin ViewModel implementation mapping backend plans to QML lists.

#include "digital_twin_viewmodel.h"

#include <QVariantMap>

namespace ume::dashboard {

DigitalTwinViewModel::DigitalTwinViewModel(MockBackend* backend, QObject* parent)
    : QObject(parent), backend_(backend) {
    connect(backend_, &MockBackend::telemetry_updated, this,
            &DigitalTwinViewModel::simulationsChanged);
}

QVariantList DigitalTwinViewModel::simulationPlans() const {
    QVariantList list;
    auto plans = backend_->get_simulation_plans();
    for (const auto& plan : plans) {
        QVariantMap map;
        map["plan_id"] = plan.plan_id;
        map["name"] = plan.name;
        map["target_tier"] = plan.target_tier;
        map["latency_us"] = plan.latency_us;
        map["bandwidth_gbps"] = plan.bandwidth_gbps;
        map["hit_ratio"] = plan.hit_ratio;
        map["score"] = plan.score;
        list.push_back(map);
    }
    return list;
}

} // namespace ume::dashboard
