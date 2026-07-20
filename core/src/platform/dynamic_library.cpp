/// @file dynamic_library.cpp
/// @brief Implementation of cross-platform dynamic library loader.

#include "ume/platform/dynamic_library.h"
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
    #include <dlfcn.h>
#endif

namespace ume::platform {

DynamicLibrary::~DynamicLibrary() {
    close();
}

DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept
    : handle_(other.handle_) {
    other.handle_ = nullptr;
}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept {
    if (this != &other) {
        close();
        handle_ = other.handle_;
        other.handle_ = nullptr;
    }
    return *this;
}

Result<DynamicLibrary> DynamicLibrary::open(const std::string& path) noexcept {
    DynamicLibrary lib;

#if defined(UME_PLATFORM_WINDOWS)
    HMODULE h = LoadLibraryA(path.c_str());
    if (!h) {
        return Status::kPluginLoadFailed;
    }
    lib.handle_ = reinterpret_cast<void*>(h);
#else
    void* h = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!h) {
        return Status::kPluginLoadFailed;
    }
    lib.handle_ = h;
#endif

    return lib;
}

void* DynamicLibrary::get_symbol_raw(const char* symbol_name) const noexcept {
    if (!handle_ || !symbol_name) return nullptr;

#if defined(UME_PLATFORM_WINDOWS)
    FARPROC proc = GetProcAddress(reinterpret_cast<HMODULE>(handle_), symbol_name);
    return reinterpret_cast<void*>(proc);
#else
    return ::dlsym(handle_, symbol_name);
#endif
}

void DynamicLibrary::close() noexcept {
    if (!handle_) return;

#if defined(UME_PLATFORM_WINDOWS)
    FreeLibrary(reinterpret_cast<HMODULE>(handle_));
#else
    ::dlclose(handle_);
#endif

    handle_ = nullptr;
}

} // namespace ume::platform
