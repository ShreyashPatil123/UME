/// @file test_dynamic_library.cpp
/// @brief Unit tests for dynamic library loader platform abstraction.

#include "ume/platform/dynamic_library.h"

#include <gtest/gtest.h>

namespace ume::platform {
namespace {

TEST(DynamicLibraryTest, OpenNonExistentLibraryReturnsError) {
    auto result = DynamicLibrary::open("non_existent_library_1234567.dll");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.status(), Status::kPluginLoadFailed);
}

TEST(DynamicLibraryTest, MoveSemantics) {
    DynamicLibrary lib1;
    EXPECT_FALSE(lib1.is_loaded());

    DynamicLibrary lib2 = std::move(lib1);
    EXPECT_FALSE(lib2.is_loaded());
}

} // namespace
} // namespace ume::platform
