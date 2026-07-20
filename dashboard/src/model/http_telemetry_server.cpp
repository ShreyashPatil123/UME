/// @file http_telemetry_server.cpp
/// @brief Telemetry REST JSON HTTP server implementation.

#include "http_telemetry_server.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

namespace ume::dashboard {

HttpTelemetryServer::HttpTelemetryServer(RealEngineAdapter* adapter, quint16 port,
                                         QObject* parent) noexcept
    : QObject(parent), adapter_(adapter), port_(port) {
    connect(&server_, &QTcpServer::newConnection, this,
            &HttpTelemetryServer::handle_new_connection);
    if (server_.listen(QHostAddress::Any, port_)) {
        uptime_timer_.start();
    }
}

void HttpTelemetryServer::handle_new_connection() noexcept {
    while (server_.hasPendingConnections()) {
        QTcpSocket* socket = server_.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &HttpTelemetryServer::read_request);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

void HttpTelemetryServer::read_request() noexcept {
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    if (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();
        QStringList tokens = line.split(" ");
        if (tokens.size() < 2)
            return;

        QString method = tokens[0];
        QString path = tokens[1];

        if (method == "GET") {
            if (path == "/health") {
                QJsonObject json;
                json["status"] = "ok";
                json["health_score"] = adapter_->get_advisor_report().health_score;
                send_response(socket, QJsonDocument(json).toJson());
            } else if (path == "/status") {
                QJsonObject json;
                json["status"] = "active";
                json["uptime_s"] = static_cast<double>(uptime_timer_.elapsed()) / 1000.0;
                send_response(socket, QJsonDocument(json).toJson());
            } else if (path == "/statistics") {
                auto stats = adapter_->get_statistics();
                QJsonObject json;
                json["current_ram_bytes"] = static_cast<double>(stats.current_ram_bytes);
                json["current_vram_bytes"] = static_cast<double>(stats.current_vram_bytes);
                json["current_ssd_bytes"] = static_cast<double>(stats.current_ssd_bytes);
                json["peak_ram_bytes"] = static_cast<double>(stats.peak_ram_bytes);
                json["peak_vram_bytes"] = static_cast<double>(stats.peak_vram_bytes);
                send_response(socket, QJsonDocument(json).toJson());
            } else if (path == "/predictions") {
                QJsonObject json;
                json["accuracy"] = adapter_->get_accuracy();
                send_response(socket, QJsonDocument(json).toJson());
            } else if (path == "/advisor") {
                auto report = adapter_->get_advisor_report();
                QJsonArray recs_arr;
                for (const auto& rec : report.recommendations) {
                    QJsonObject rec_obj;
                    rec_obj["category"] = QString::fromStdString(rec.category);
                    rec_obj["severity"] = static_cast<int>(rec.severity);
                    rec_obj["description"] = QString::fromStdString(rec.description);
                    rec_obj["root_cause"] = QString::fromStdString(rec.root_cause);
                    recs_arr.append(rec_obj);
                }
                QJsonObject json;
                json["health_score"] = report.health_score;
                json["recommendations"] = recs_arr;
                send_response(socket, QJsonDocument(json).toJson());
            } else if (path == "/applications") {
                QJsonArray apps_arr;
                QJsonObject app1;
                app1["name"] = "PyTorch NLP Model";
                app1["vram_mb"] = 4096.0;
                app1["ram_mb"] = 8192.0;
                apps_arr.append(app1);
                send_response(socket, QJsonDocument(apps_arr).toJson());
            } else {
                send_404(socket);
            }
        } else {
            send_404(socket);
        }
    }
}

void HttpTelemetryServer::send_response(QTcpSocket* socket, const QByteArray& body,
                                        const QByteArray& content_type) noexcept {
    QByteArray header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: " +
        content_type +
        "\r\n"
        "Content-Length: " +
        QByteArray::number(body.size()) +
        "\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n\r\n";
    socket->write(header);
    socket->write(body);
    socket->flush();
    socket->disconnectFromHost();
}

void HttpTelemetryServer::send_404(QTcpSocket* socket) noexcept {
    QByteArray body = "{\"error\": \"not found\"}";
    QByteArray header =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " +
        QByteArray::number(body.size()) +
        "\r\n"
        "Connection: close\r\n\r\n";
    socket->write(header);
    socket->write(body);
    socket->flush();
    socket->disconnectFromHost();
}

} // namespace ume::dashboard
