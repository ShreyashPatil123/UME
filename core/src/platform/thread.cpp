/// @file thread.cpp
/// @brief Implementation of cross-platform thread management.

#include "ume/platform/thread.h"
#include "ume/platform/platform.h"

#include <thread>

#if defined(UME_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#elif defined(UME_PLATFORM_LINUX) || defined(UME_PLATFORM_MACOS)
    #include <pthread.h>
    #include <sched.h>
#endif

namespace ume::platform {

Status set_current_thread_name(const char* name) noexcept {
    if (!name) return Status::kInvalidArgument;

#if defined(UME_PLATFORM_WINDOWS)
    wchar_t wname[64];
    size_t converted = 0;
    mbstowcs_s(&converted, wname, name, 63);
    HRESULT hr = SetThreadDescription(GetCurrentThread(), wname);
    return SUCCEEDED(hr) ? Status::kOk : Status::kInternalError;
#elif defined(UME_PLATFORM_LINUX)
    int rc = pthread_setname_np(pthread_self(), name);
    return (rc == 0) ? Status::kOk : Status::kInternalError;
#elif defined(UME_PLATFORM_MACOS)
    int rc = pthread_setname_np(name);
    return (rc == 0) ? Status::kOk : Status::kInternalError;
#else
    return Status::kUnimplemented;
#endif
}

Status set_current_thread_affinity(size_t cpu_core) noexcept {
    size_t total_cores = get_hardware_concurrency();
    if (cpu_core >= total_cores) {
        return Status::kInvalidArgument;
    }

#if defined(UME_PLATFORM_WINDOWS)
    DWORD_PTR mask = static_cast<DWORD_PTR>(1ULL << cpu_core);
    DWORD_PTR prev = SetThreadAffinityMask(GetCurrentThread(), mask);
    return (prev != 0) ? Status::kOk : Status::kInternalError;
#elif defined(UME_PLATFORM_LINUX)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(static_cast<int>(cpu_core), &cpuset);
    int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    return (rc == 0) ? Status::kOk : Status::kInternalError;
#else
    return Status::kUnimplemented;
#endif
}

Status set_current_thread_priority(ThreadPriority priority) noexcept {
#if defined(UME_PLATFORM_WINDOWS)
    int win_prio = THREAD_PRIORITY_NORMAL;
    switch (priority) {
        case ThreadPriority::kIdle: win_prio = THREAD_PRIORITY_IDLE; break;
        case ThreadPriority::kLow: win_prio = THREAD_PRIORITY_BELOW_NORMAL; break;
        case ThreadPriority::kNormal: win_prio = THREAD_PRIORITY_NORMAL; break;
        case ThreadPriority::kHigh: win_prio = THREAD_PRIORITY_ABOVE_NORMAL; break;
        case ThreadPriority::kRealtime: win_prio = THREAD_PRIORITY_TIME_CRITICAL; break;
    }
    BOOL ok = SetThreadPriority(GetCurrentThread(), win_prio);
    return ok ? Status::kOk : Status::kInternalError;
#elif defined(UME_PLATFORM_LINUX)
    int policy = SCHED_OTHER;
    struct sched_param param{};
    if (priority == ThreadPriority::kRealtime) {
        policy = SCHED_FIFO;
        param.sched_priority = 10;
    } else if (priority == ThreadPriority::kHigh) {
        param.sched_priority = 0; // Standard niceness adjusted if root
    }
    int rc = pthread_setschedparam(pthread_self(), policy, &param);
    return (rc == 0) ? Status::kOk : Status::kInternalError;
#else
    return Status::kUnimplemented;
#endif
}

size_t get_hardware_concurrency() noexcept {
    unsigned int count = std::thread::hardware_concurrency();
    return (count > 0) ? static_cast<size_t>(count) : 1;
}

} // namespace ume::platform
