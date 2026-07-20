/// @file main.cpp
/// @brief Main entry point for the UME Dashboard initialized with real engine telemetry.

#include "advisor_viewmodel.h"
#include "dashboard_viewmodel.h"
#include "digital_twin_viewmodel.h"
#include "http_telemetry_server.h"
#include "memory_viewmodel.h"
#include "real_engine_adapter.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("Unified Memory Engine Dashboard");
    app.setOrganizationName("UME Contributors");
    app.setApplicationVersion("1.0.0");

    QQmlApplicationEngine engine;

    // Instantiate real UME engine adapter and lightweight REST HTTP telemetry server
    ume::dashboard::RealEngineAdapter backend;
    ume::dashboard::HttpTelemetryServer telemetry_server(&backend, 8082);

    // Instantiate ViewModels linked to real engine adapter
    ume::dashboard::DashboardViewModel dashboard_vm(&backend);
    ume::dashboard::MemoryViewModel memory_vm(&backend);
    ume::dashboard::AdvisorViewModel advisor_vm(&backend);
    ume::dashboard::DigitalTwinViewModel digital_twin_vm(&backend);

    // Register ViewModels into QML context
    auto* ctx = engine.rootContext();
    ctx->setContextProperty("dashboardVM", &dashboard_vm);
    ctx->setContextProperty("memoryVM", &memory_vm);
    ctx->setContextProperty("advisorVM", &advisor_vm);
    ctx->setContextProperty("digitalTwinVM", &digital_twin_vm);

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
