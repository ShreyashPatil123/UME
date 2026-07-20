/// @file main.cpp
/// @brief Main entry point for the UME Professional Desktop Dashboard.

#include "advisor_viewmodel.h"
#include "dashboard_viewmodel.h"
#include "digital_twin_viewmodel.h"
#include "memory_viewmodel.h"
#include "mock_backend.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("Unified Memory Engine Dashboard");
    app.setOrganizationName("UME Contributors");
    app.setApplicationVersion("1.0.0");

    QQmlApplicationEngine engine;

    // Instantiate models and viewmodels
    ume::dashboard::MockBackend backend;
    ume::dashboard::DashboardViewModel dashboard_vm(&backend);
    ume::dashboard::MemoryViewModel memory_vm(&backend);
    ume::dashboard::AdvisorViewModel advisor_vm(&backend);
    ume::dashboard::DigitalTwinViewModel digital_twin_vm(&backend);

    // Register viewmodels into QML context
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
