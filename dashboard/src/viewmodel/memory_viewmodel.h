#pragma once

/// @file memory_viewmodel.h
/// @brief ViewModel providing application memory profiles and active listings.

#include "mock_backend.h"

#include <QObject>
#include <QVariantList>

namespace ume::dashboard {

class MemoryViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList applications READ applications NOTIFY appsChanged)

public:
    explicit MemoryViewModel(MockBackend* backend, QObject* parent = nullptr);
    ~MemoryViewModel() override = default;

    QVariantList applications() const;

signals:
    void appsChanged();

private:
    MockBackend* backend_;
};

} // namespace ume::dashboard
