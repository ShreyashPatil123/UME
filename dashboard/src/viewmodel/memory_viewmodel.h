#pragma once

/// @file memory_viewmodel.h
/// @brief ViewModel providing application profiles mapped from real engine.

#include "real_engine_adapter.h"

#include <QObject>
#include <QVariantList>

namespace ume::dashboard {

class MemoryViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList applications READ applications NOTIFY appsChanged)

public:
    explicit MemoryViewModel(RealEngineAdapter* backend, QObject* parent = nullptr) noexcept;
    ~MemoryViewModel() override = default;

    QVariantList applications() const noexcept;

signals:
    void appsChanged();

private:
    RealEngineAdapter* backend_;
};

} // namespace ume::dashboard
