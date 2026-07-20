#pragma once

/// @file digital_twin_viewmodel.h
/// @brief ViewModel providing simulated candidate strategies from digital twin engine.

#include "real_engine_adapter.h"

#include <QObject>
#include <QVariantList>

namespace ume::dashboard {

class DigitalTwinViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList simulationPlans READ simulationPlans NOTIFY simulationsChanged)

public:
    explicit DigitalTwinViewModel(RealEngineAdapter* backend, QObject* parent = nullptr) noexcept;
    ~DigitalTwinViewModel() override = default;

    QVariantList simulationPlans() const noexcept;

signals:
    void simulationsChanged();

private:
    RealEngineAdapter* backend_;
};

} // namespace ume::dashboard
