/// @file digital_twin_viewmodel.cpp
/// @brief DigitalTwin ViewModel mapping candidate simulations from real twin engine.

#include "digital_twin_viewmodel.h"

#include <QVariantMap>

namespace ume::dashboard {

DigitalTwinViewModel::DigitalTwinViewModel(RealEngineAdapter* backend, QObject* parent) noexcept
    : QObject(parent), backend_(backend) {
    connect(backend_, &RealEngineAdapter::data_changed, this,
            &DigitalTwinViewModel::simulationsChanged);
}

QVariantList DigitalTwinViewModel::simulationPlans() const noexcept {
    QVariantList list;
    auto plans = backend_->get_evaluated_plans();
    for (const auto& plan : plans) {
        QVariantMap map;
        map["plan_id"] = static_cast<int>(plan.plan_id);
        map["name"] = QString::fromStdString(plan.name);

        QString tier_str = "RAM";
        switch (plan.target_tier) {
            case event::TierClass::kVram:
                tier_str = "VRAM";
                break;
            case event::TierClass::kNvme:
                tier_str = "SSD";
                break;
            default:
                break;
        }
        map["target_tier"] = tier_str;
        map["latency_us"] = static_cast<double>(plan.estimated_latency_us);
        map["bandwidth_gbps"] =
            static_cast<double>(plan.estimated_bandwidth_bytes_sec) / (1024.0 * 1024.0 * 1024.0);
        map["hit_ratio"] = plan.cache_hit_ratio;
        map["score"] = plan.overall_score;
        list.push_back(map);
    }
    return list;
}

} // namespace ume::dashboard
