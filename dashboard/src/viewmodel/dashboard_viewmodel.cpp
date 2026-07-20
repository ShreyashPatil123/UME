/// @file dashboard_viewmodel.cpp
/// @brief Dashboard ViewModel implementation listening to RealEngineAdapter.

#include "dashboard_viewmodel.h"

namespace ume::dashboard {

DashboardViewModel::DashboardViewModel(RealEngineAdapter* backend, QObject* parent) noexcept
    : QObject(parent), backend_(backend) {
    connect(backend_, &RealEngineAdapter::data_changed, this,
            &DashboardViewModel::telemetryChanged);
}

} // namespace ume::dashboard
