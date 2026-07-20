# dashboard/CMakeLists.txt
# CMake configuration for UME Real Engine Integrated Dashboard (UI).
# Part of UME Milestone M9 (Task T017).

cmake_minimum_required(VERSION 3.28)
project(ume_dashboard LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Quick Charts Network WebSockets REQUIRED)

add_executable(ume_dashboard
    src/main.cpp
    src/viewmodel/dashboard_viewmodel.cpp
    src/viewmodel/memory_viewmodel.cpp
    src/viewmodel/advisor_viewmodel.cpp
    src/viewmodel/digital_twin_viewmodel.cpp
    src/model/real_engine_adapter.cpp
    src/model/http_telemetry_server.cpp
    resources.qrc
)

target_include_directories(ume_dashboard
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodel
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model
)

target_link_libraries(ume_dashboard
    PRIVATE
        ume_core
        ume_common
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Charts
        Qt6::Network
        Qt6::WebSockets
)

# Deployment and runtime configurations
set_target_properties(ume_dashboard PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
