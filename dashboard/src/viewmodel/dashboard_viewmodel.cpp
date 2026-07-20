/// @file dashboard_viewmodel.cpp
/// @brief Dashboard ViewModel implementation forwarding telemetry updates.

#include "dashboard_viewmodel.h"

namespace ume::dashboard {

DashboardViewModel::DashboardViewModel(MockBackend* backend, QObject* parent)
    : QObject(parent), backend_(backend) {
    connect(backend_, &MockBackend::telemetry_updated, this, &DashboardViewModel::telemetryChanged);
}

} // namespace ume::dashboard
