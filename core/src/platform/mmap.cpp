/// @file mmap.cpp
/// @brief Implementation of memory-mapped file abstraction.

#include "ume/platform/mmap.h"

#include "ume/platform/platform.h"

#include <utility>

#if defined(UME_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace ume::platform {

MmapFile::~MmapFile() {
    close();
}

MmapFile::MmapFile(MmapFile&& other) noexcept : data_(other.data_), size_(other.size_) {
    other.data_ = nullptr;
    other.size_ = 0;
#if defined(UME_PLATFORM_WINDOWS)
    file_handle_ = other.file_handle_;
    mapping_handle_ = other.mapping_handle_;
    other.file_handle_ = nullptr;
    other.mapping_handle_ = nullptr;
#else
    fd_ = other.fd_;
    other.fd_ = -1;
#endif
}

MmapFile& MmapFile::operator=(MmapFile&& other) noexcept {
    if (this != &other) {
        close();
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
#if defined(UME_PLATFORM_WINDOWS)
        file_handle_ = other.file_handle_;
        mapping_handle_ = other.mapping_handle_;
        other.file_handle_ = nullptr;
        other.mapping_handle_ = nullptr;
#else
        fd_ = other.fd_;
        other.fd_ = -1;
#endif
    }
    return *this;
}

Status MmapFile::open_read_write(const std::string& path, uint64_t initial_size_bytes) noexcept {
    close();

#if defined(UME_PLATFORM_WINDOWS)
    HANDLE hFile =
        CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return Status::kFileNotFound;
    }

    LARGE_INTEGER size_li;
    size_li.QuadPart = static_cast<LONGLONG>(initial_size_bytes);

    HANDLE hMapping =
        CreateFileMappingA(hFile, NULL, PAGE_READWRITE, size_li.HighPart, size_li.LowPart, NULL);

    if (hMapping == NULL) {
        CloseHandle(hFile);
        return Status::kIoError;
    }

    void* ptr =
        MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<SIZE_T>(initial_size_bytes));
    if (ptr == NULL) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return Status::kOutOfMemory;
    }

    file_handle_ = hFile;
    mapping_handle_ = hMapping;
    data_ = static_cast<uint8_t*>(ptr);
    size_ = initial_size_bytes;
    return Status::kOk;
#else
    int fd = ::open(path.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        return Status::kFileNotFound;
    }

    if (::ftruncate(fd, static_cast<off_t>(initial_size_bytes)) != 0) {
        ::close(fd);
        return Status::kIoError;
    }

    void* ptr = ::mmap(nullptr, initial_size_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        ::close(fd);
        return Status::kOutOfMemory;
    }

    fd_ = fd;
    data_ = static_cast<uint8_t*>(ptr);
    size_ = initial_size_bytes;
    return Status::kOk;
#endif
}

Status MmapFile::open_read_only(const std::string& path) noexcept {
    close();

#if defined(UME_PLATFORM_WINDOWS)
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return Status::kFileNotFound;
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(hFile, &file_size)) {
        CloseHandle(hFile);
        return Status::kIoError;
    }

    HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (hMapping == NULL) {
        CloseHandle(hFile);
        return Status::kIoError;
    }

    void* ptr = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (ptr == NULL) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return Status::kOutOfMemory;
    }

    file_handle_ = hFile;
    mapping_handle_ = hMapping;
    data_ = static_cast<uint8_t*>(ptr);
    size_ = static_cast<uint64_t>(file_size.QuadPart);
    return Status::kOk;
#else
    int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        return Status::kFileNotFound;
    }

    struct stat st;
    if (::fstat(fd, &st) != 0) {
        ::close(fd);
        return Status::kIoError;
    }

    uint64_t file_size = static_cast<uint64_t>(st.st_size);
    void* ptr = ::mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        ::close(fd);
        return Status::kOutOfMemory;
    }

    fd_ = fd;
    data_ = static_cast<uint8_t*>(ptr);
    size_ = file_size;
    return Status::kOk;
#endif
}

Status MmapFile::flush() noexcept {
    if (!data_)
        return Status::kOk;

#if defined(UME_PLATFORM_WINDOWS)
    BOOL ok = FlushViewOfFile(data_, static_cast<SIZE_T>(size_));
    return ok ? Status::kOk : Status::kIoError;
#else
    int rc = ::msync(data_, size_, MS_SYNC);
    return (rc == 0) ? Status::kOk : Status::kIoError;
#endif
}

void MmapFile::close() noexcept {
    if (!data_)
        return;

#if defined(UME_PLATFORM_WINDOWS)
    UnmapViewOfFile(data_);
    if (mapping_handle_)
        CloseHandle(mapping_handle_);
    if (file_handle_)
        CloseHandle(file_handle_);
    mapping_handle_ = nullptr;
    file_handle_ = nullptr;
#else
    ::munmap(data_, size_);
    if (fd_ >= 0)
        ::close(fd_);
    fd_ = -1;
#endif

    data_ = nullptr;
    size_ = 0;
}

} // namespace ume::platform
