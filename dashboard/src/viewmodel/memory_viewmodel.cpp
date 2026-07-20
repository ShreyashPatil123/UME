/// @file memory_viewmodel.cpp
/// @brief Memory ViewModel implementation converting backend models to QML maps.

#include "memory_viewmodel.h"

#include <QVariantMap>

namespace ume::dashboard {

MemoryViewModel::MemoryViewModel(MockBackend* backend, QObject* parent)
    : QObject(parent), backend_(backend) {
    connect(backend_, &MockBackend::telemetry_updated, this, &MemoryViewModel::appsChanged);
}

QVariantList MemoryViewModel::applications() const {
    QVariantList list;
    auto apps = backend_->get_applications();
    for (const auto& app : apps) {
        QVariantMap map;
        map["name"] = app.name;
        map["vram_mb"] = app.vram_mb;
        map["ram_mb"] = app.ram_mb;
        map["ssd_mb"] = app.ssd_mb;
        map["bandwidth_gbps"] = app.bandwidth_gbps;
        map["migration_count"] = app.migration_count;
        map["score"] = app.score;
        list.push_back(map);
    }
    return list;
}

} // namespace ume::dashboard
