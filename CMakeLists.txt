cmake_minimum_required(VERSION 3.28)
project(ume VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(UME_BUILD_TESTS "Build UME tests" ON)
option(UME_BUILD_BENCHMARKS "Build UME benchmarks" OFF)
option(UME_BUILD_EXAMPLES "Build UME examples" OFF)
option(UME_BUILD_DASHBOARD "Build UME dashboard" OFF)
option(UME_BUILD_PYTHON "Build UME python bindings" OFF)
option(UME_BUILD_RUST "Build UME rust bindings" OFF)
option(UME_ENABLE_CUDA "Enable CUDA support" OFF)
option(UME_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(UME_BUILD_PLUGINS "Build UME plugins" OFF)
option(UME_BUILD_TELEMETRY "Build UME telemetry" OFF)

if(UME_BUILD_TESTS)
    enable_testing()
endif()

# Compiler Warnings
function(ume_set_project_warnings project_name)
    set(MSVC_WARNINGS
        /W4
        /wd4251 # needs to have dll-interface to be used by clients of class
    )
    
    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )

    if(MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    else()
        message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif()

    target_compile_options(${project_name} INTERFACE ${PROJECT_WARNINGS})
endfunction()

# Sanitizers
if(UME_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    elseif(MSVC)
        add_compile_options(/fsanitize=address)
    endif()
endif()

# Dependencies
include(FetchContent)

# Fetch gtest if tests enabled
if(UME_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Fetch benchmark if benchmarks enabled
if(UME_BUILD_BENCHMARKS)
    set(HAVE_STD_REGEX ON CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.3
    )
    FetchContent_MakeAvailable(googlebenchmark)
endif()

# Try find_package first, fallback to FetchContent
find_package(fmt CONFIG QUIET)
if(NOT fmt_FOUND)
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG        10.2.1
    )
    FetchContent_MakeAvailable(fmt)
endif()

find_package(spdlog CONFIG QUIET)
if(NOT spdlog_FOUND)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.13.0
    )
    FetchContent_MakeAvailable(spdlog)
endif()

find_package(toml11 CONFIG QUIET)
if(NOT toml11_FOUND)
    FetchContent_Declare(
        toml11
        GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
        GIT_TAG        v3.8.1
    )
    FetchContent_MakeAvailable(toml11)
endif()

# Setup configured version header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include/ume/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/include/ume/version.h
    @ONLY
)

# Common interface library for basic project settings
add_library(ume_common INTERFACE)
ume_set_project_warnings(ume_common)
target_include_directories(ume_common INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core/include>
)

# Target link libraries for common interface
if(TARGET fmt::fmt)
    target_link_libraries(ume_common INTERFACE fmt::fmt)
elseif(TARGET fmt)
    target_link_libraries(ume_common INTERFACE fmt)
endif()

if(TARGET spdlog::spdlog)
    target_link_libraries(ume_common INTERFACE spdlog::spdlog)
elseif(TARGET spdlog)
    target_link_libraries(ume_common INTERFACE spdlog)
endif()

if(TARGET toml11::toml11)
    target_link_libraries(ume_common INTERFACE toml11::toml11)
elseif(TARGET toml11)
    target_link_libraries(ume_common INTERFACE toml11)
endif()

# Add Subdirectories
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/core)
    add_subdirectory(core)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/runtime)
    add_subdirectory(runtime)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/drivers)
    add_subdirectory(drivers)
endif()

if(UME_BUILD_PLUGINS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
    add_subdirectory(plugins)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/simulator)
    add_subdirectory(simulator)
endif()

if(UME_BUILD_TELEMETRY AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/telemetry)
    add_subdirectory(telemetry)
endif()

if(UME_BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    add_subdirectory(tests)
endif()

if(UME_BUILD_BENCHMARKS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
    add_subdirectory(benchmarks)
endif()

if(UME_BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()

if(UME_BUILD_DASHBOARD AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dashboard)
    add_subdirectory(dashboard)
endif()

# Installation
include(GNUInstallDirs)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/core/include/ume
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/include/ume/version.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ume
)
