# core/CMakeLists.txt
# UME Core Library — domain-agnostic kernel containing all fundamental data structures.

add_library(ume_core STATIC)

target_sources(ume_core
    PRIVATE
        # Config
        src/config.cpp

        # Platform
        src/platform/clock.cpp
        src/platform/thread.cpp
        src/platform/mmap.cpp
        src/platform/dynamic_library.cpp

        # Concurrency
        src/concurrency/epoch_rcu.cpp
        src/concurrency/string_interner.cpp

        # Event & Journal
        src/event/event_serializer.cpp
        src/event/journal_writer.cpp
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
else()
    target_link_libraries(ume_core PRIVATE dl pthread)
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
        tests/test_thread.cpp
        tests/test_mmap.cpp
        tests/test_dynamic_library.cpp
        tests/test_mpsc_queue.cpp
        tests/test_spsc_queue.cpp
        tests/test_epoch_rcu.cpp
        tests/test_hazard_pointers.cpp
        tests/test_string_interner.cpp
        tests/test_event.cpp
        tests/test_event_serializer.cpp
        tests/test_journal_writer.cpp
    )

    target_link_libraries(ume_core_tests
        PRIVATE
            ume_core
            GTest::gtest
            GTest::gtest_main
    )

    add_test(NAME ume_core_tests COMMAND ume_core_tests)

    add_custom_target(run_core_tests
        COMMAND ume_core_tests
        DEPENDS ume_core_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
