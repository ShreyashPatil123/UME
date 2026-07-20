# core/CMakeLists.txt
# UME Core Library — domain-agnostic kernel containing all fundamental data structures.

add_library(ume_core STATIC)

target_sources(ume_core
    PRIVATE
        # Config
        src/config.cpp

        # Platform (cross-platform selected at compile time via #ifdef)
        src/platform/clock.cpp
)

target_include_directories(ume_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(ume_core
    PUBLIC
        ume_common
)

# Platform-specific link libraries
if(WIN32)
    target_link_libraries(ume_core PRIVATE winmm)
endif()

# Set compiler definitions
target_compile_definitions(ume_core
    PRIVATE
        $<$<CONFIG:Debug>:UME_DEBUG>
)

# Export for install
set_target_properties(ume_core PROPERTIES
    EXPORT_NAME core
    OUTPUT_NAME ume_core
    POSITION_INDEPENDENT_CODE ON
)

# ── Unit Tests ──
if(UME_BUILD_TESTS)
    add_executable(ume_core_tests
        tests/test_types.cpp
        tests/test_status.cpp
        tests/test_config.cpp
        tests/test_platform.cpp
    )

    target_link_libraries(ume_core_tests
        PRIVATE
            ume_core
            GTest::gtest
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(ume_core_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        PROPERTIES
            TIMEOUT 30
    )
endif()
