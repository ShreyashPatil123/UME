#pragma once

/// @file http_telemetry_server.h
/// @brief Lightweight HTTP server exposing real UME engine stats over REST JSON.

#include "real_engine_adapter.h"

#include <QElapsedTimer>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

namespace ume::dashboard {

class HttpTelemetryServer : public QObject {
    Q_OBJECT
public:
    explicit HttpTelemetryServer(RealEngineAdapter* adapter, quint16 port = 8082,
                                 QObject* parent = nullptr) noexcept;
    ~HttpTelemetryServer() override = default;

    quint16 port() const noexcept { return port_; }

private slots:
    void handle_new_connection() noexcept;
    void read_request() noexcept;

private:
    void send_response(QTcpSocket* socket, const QByteArray& body,
                       const QByteArray& content_type = "application/json") noexcept;
    void send_404(QTcpSocket* socket) noexcept;

    QTcpServer server_;
    RealEngineAdapter* adapter_;
    quint16 port_;
    QElapsedTimer uptime_timer_;
};

} // namespace ume::dashboard
