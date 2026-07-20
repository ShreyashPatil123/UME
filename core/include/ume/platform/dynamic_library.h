#pragma once

/// @file dynamic_library.h
/// @brief Cross-platform dynamic library loader (DynamicLibrary).
///
/// Wraps LoadLibraryA/GetProcAddress/FreeLibrary on Windows and
/// dlopen/dlsym/dlclose on Linux/macOS for loading UME plugin DLLs/.so files.

#include "ume/status.h"
#include "ume/types.h"

#include <string>

namespace ume::platform {

/// @brief Cross-platform dynamic library loader handle.
class DynamicLibrary {
public:
    DynamicLibrary() noexcept = default;
    ~DynamicLibrary();

    // Non-copyable
    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;

    // Moveable
    DynamicLibrary(DynamicLibrary&& other) noexcept;
    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept;

    /// @brief Open and load a shared library (.dll / .so).
    ///
    /// @param path File system path to shared library.
    /// @return Result<DynamicLibrary> containing loaded instance or Status::kPluginLoadFailed.
    [[nodiscard]] static Result<DynamicLibrary> open(const std::string& path) noexcept;

    /// @brief Look up symbol address by string name.
    ///
    /// @tparam T Function pointer or pointer type.
    /// @param symbol_name Name of exported symbol.
    /// @return Pointer to symbol, or nullptr if not found.
    template <typename T>
    [[nodiscard]] T get_symbol(const char* symbol_name) const noexcept {
        return reinterpret_cast<T>(get_symbol_raw(symbol_name));
    }

    /// @brief Unload library and release handles.
    void close() noexcept;

    /// @brief Returns true if library is currently loaded.
    [[nodiscard]] bool is_loaded() const noexcept { return handle_ != nullptr; }

private:
    [[nodiscard]] void* get_symbol_raw(const char* symbol_name) const noexcept;

    void* handle_{nullptr};
};

} // namespace ume::platform
