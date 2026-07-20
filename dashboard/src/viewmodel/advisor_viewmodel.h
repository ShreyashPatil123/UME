#pragma once

/// @file advisor_viewmodel.h
/// @brief ViewModel providing memory advisor recommendations from real advisor engine.

#include "real_engine_adapter.h"

#include <QObject>
#include <QVariantList>

namespace ume::dashboard {

class AdvisorViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList recommendations READ recommendations NOTIFY recommendationsChanged)

public:
    explicit AdvisorViewModel(RealEngineAdapter* backend, QObject* parent = nullptr) noexcept;
    ~AdvisorViewModel() override = default;

    QVariantList recommendations() const noexcept;

signals:
    void recommendationsChanged();

private:
    RealEngineAdapter* backend_;
};

} // namespace ume::dashboard
