/// @file test_mmap.cpp
/// @brief Unit tests for memory-mapped file abstraction.

#include "ume/platform/mmap.h"

#include <cstdio>
#include <gtest/gtest.h>

namespace ume::platform {
namespace {

TEST(MmapTest, OpenReadWriteAndClose) {
    const std::string test_path = "test_mmap_rw.tmp";

    MmapFile file;
    EXPECT_FALSE(file.is_mapped());

    Status status = file.open_read_write(test_path, 4096);
    EXPECT_TRUE(is_ok(status));
    EXPECT_TRUE(file.is_mapped());
    EXPECT_EQ(file.size(), 4096ULL);
    EXPECT_NE(file.data(), nullptr);

    // Write patterns to mapped memory
    uint8_t* ptr = file.data();
    ptr[0] = 0xDE;
    ptr[1] = 0xAD;
    ptr[2] = 0xBE;
    ptr[3] = 0xEF;

    EXPECT_TRUE(is_ok(file.flush()));
    file.close();
    EXPECT_FALSE(file.is_mapped());

    // Clean up file
    std::remove(test_path.c_str());
}

TEST(MmapTest, OpenReadOnly) {
    const std::string test_path = "test_mmap_ro.tmp";

    // Create file first
    {
        MmapFile file;
        ASSERT_TRUE(is_ok(file.open_read_write(test_path, 1024)));
        file.data()[0] = 42;
        file.flush();
    }

    // Open read only
    MmapFile ro_file;
    Status status = ro_file.open_read_only(test_path);
    EXPECT_TRUE(is_ok(status));
    EXPECT_TRUE(ro_file.is_mapped());
    EXPECT_EQ(ro_file.data()[0], 42);

    ro_file.close();
    std::remove(test_path.c_str());
}

TEST(MmapTest, MoveSemantics) {
    const std::string test_path = "test_mmap_move.tmp";

    MmapFile file1;
    ASSERT_TRUE(is_ok(file1.open_read_write(test_path, 2048)));
    uint8_t* orig_data = file1.data();

    MmapFile file2 = std::move(file1);
    EXPECT_FALSE(file1.is_mapped());
    EXPECT_TRUE(file2.is_mapped());
    EXPECT_EQ(file2.data(), orig_data);

    file2.close();
    std::remove(test_path.c_str());
}

} // namespace
} // namespace ume::platform
