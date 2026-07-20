#pragma once

/// @file digital_twin_viewmodel.h
/// @brief ViewModel providing simulated candidate strategies and evaluations.

#include "mock_backend.h"

#include <QObject>
#include <QVariantList>

namespace ume::dashboard {

class DigitalTwinViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList simulationPlans READ simulationPlans NOTIFY simulationsChanged)

public:
    explicit DigitalTwinViewModel(MockBackend* backend, QObject* parent = nullptr);
    ~DigitalTwinViewModel() override = default;

    QVariantList simulationPlans() const;

signals:
    void simulationsChanged();

private:
    MockBackend* backend_;
};

} // namespace ume::dashboard
