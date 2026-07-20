/// @file advisor_viewmodel.cpp
/// @brief Advisor ViewModel mapping advice notifications from real engine.

#include "advisor_viewmodel.h"

#include <QVariantMap>

namespace ume::dashboard {

AdvisorViewModel::AdvisorViewModel(RealEngineAdapter* backend, QObject* parent) noexcept
    : QObject(parent), backend_(backend) {
    connect(backend_, &RealEngineAdapter::data_changed, this,
            &AdvisorViewModel::recommendationsChanged);
}

QVariantList AdvisorViewModel::recommendations() const noexcept {
    QVariantList list;
    auto report = backend_->get_advisor_report();
    for (const auto& rec : report.recommendations) {
        QVariantMap map;
        map["category"] = QString::fromStdString(rec.category);

        QString severity_str = "INFO";
        switch (rec.severity) {
            case event::AdvisorSeverity::kCritical:
                severity_str = "CRITICAL";
                break;
            case event::AdvisorSeverity::kHigh:
                severity_str = "HIGH";
                break;
            case event::AdvisorSeverity::kWarning:
                severity_str = "WARNING";
                break;
            default:
                break;
        }
        map["severity"] = severity_str;
        map["description"] = QString::fromStdString(rec.description);
        map["root_cause"] = QString::fromStdString(rec.root_cause);
        map["suggested_action"] = QString::fromStdString(rec.suggested_action);
        map["estimated_benefit_us"] = static_cast<double>(rec.estimated_benefit_us);
        map["confidence"] = rec.confidence_score;
        map["health_score"] = report.health_score;
        list.push_back(map);
    }
    return list;
}

} // namespace ume::dashboard
