#pragma once

/// @file mmap.h
/// @brief Cross-platform memory-mapped file abstraction (MmapFile).
///
/// Provides portable memory-mapped file operations for Windows and Linux.
/// Used by the Event Journal to manage 64 MB append-only log segments.

#include "ume/platform/platform.h"
#include "ume/status.h"
#include "ume/types.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace ume::platform {

/// @brief Cross-platform memory-mapped file wrapper.
///
/// Move-only RAII handle to a mapped file region.
class MmapFile {
public:
    MmapFile() noexcept = default;
    ~MmapFile();

    // Non-copyable
    MmapFile(const MmapFile&) = delete;
    MmapFile& operator=(const MmapFile&) = delete;

    // Moveable
    MmapFile(MmapFile&& other) noexcept;
    MmapFile& operator=(MmapFile&& other) noexcept;

    /// @brief Create or open file and map as read/write with specified size.
    ///
    /// @param path File system path.
    /// @param initial_size_bytes Requested size of file mapping.
    /// @return Status::kOk on success, error status otherwise.
    Status open_read_write(const std::string& path, uint64_t initial_size_bytes) noexcept;

    /// @brief Open existing file and map as read-only.
    ///
    /// @param path File system path.
    /// @return Status::kOk on success, error status otherwise.
    Status open_read_only(const std::string& path) noexcept;

    /// @brief Flush modified pages to disk storage.
    /// @return Status::kOk on success.
    Status flush() noexcept;

    /// @brief Unmap memory region and close OS file handles.
    void close() noexcept;

    /// @brief Mutable pointer to mapped bytes.
    [[nodiscard]] uint8_t* data() noexcept { return data_; }

    /// @brief Const pointer to mapped bytes.
    [[nodiscard]] const uint8_t* data() const noexcept { return data_; }

    /// @brief Mapped region size in bytes.
    [[nodiscard]] uint64_t size() const noexcept { return size_; }

    /// @brief Returns true if file is currently mapped.
    [[nodiscard]] bool is_mapped() const noexcept { return data_ != nullptr; }

private:
    uint8_t* data_{nullptr};
    uint64_t size_{0};

#if defined(UME_PLATFORM_WINDOWS)
    void* file_handle_{nullptr};    // HANDLE
    void* mapping_handle_{nullptr}; // HANDLE
#else
    int fd_{-1};
#endif
};

} // namespace ume::platform
