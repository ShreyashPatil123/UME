/// @file advisor_viewmodel.cpp
/// @brief Advisor ViewModel implementation mapping backend recommendations to variant objects.

#include "advisor_viewmodel.h"

#include <QVariantMap>

namespace ume::dashboard {

AdvisorViewModel::AdvisorViewModel(MockBackend* backend, QObject* parent)
    : QObject(parent), backend_(backend) {
    connect(backend_, &MockBackend::telemetry_updated, this,
            &AdvisorViewModel::recommendationsChanged);
}

QVariantList AdvisorViewModel::recommendations() const {
    QVariantList list;
    auto recs = backend_->get_recommendations();
    for (const auto& rec : recs) {
        QVariantMap map;
        map["category"] = rec.category;
        map["severity"] = rec.severity;
        map["description"] = rec.description;
        map["root_cause"] = rec.root_cause;
        map["suggested_action"] = rec.suggested_action;
        map["estimated_benefit_us"] = rec.estimated_benefit_us;
        map["confidence"] = rec.confidence;
        map["health_score"] = rec.health_score;
        list.push_back(map);
    }
    return list;
}

} // namespace ume::dashboard
