#pragma once

/// @file advisor_viewmodel.h
/// @brief ViewModel providing human-actionable recommendations and health indices.

#include "mock_backend.h"

#include <QObject>
#include <QVariantList>

namespace ume::dashboard {

class AdvisorViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList recommendations READ recommendations NOTIFY recommendationsChanged)

public:
    explicit AdvisorViewModel(MockBackend* backend, QObject* parent = nullptr);
    ~AdvisorViewModel() override = default;

    QVariantList recommendations() const;

signals:
    void recommendationsChanged();

private:
    MockBackend* backend_;
};

} // namespace ume::dashboard
