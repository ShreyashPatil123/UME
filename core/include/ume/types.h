#pragma once

/// @file types.h
/// @brief Core type definitions for the Unified Memory Engine.
///
/// This header defines the fundamental scalar types used across all UME modules.
/// These types are part of the frozen v3 architecture specification. Changes
/// require an RFC with full architectural review.

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>

namespace ume {

// ──────────────────────────────────────────────────────────────────────
// Identity Types
// ──────────────────────────────────────────────────────────────────────

/// @brief Globally unique identifier for a tracked memory object.
///
/// ObjectIds are monotonically increasing 64-bit integers assigned by the
/// Event Journal writer thread. They are never reused. An ObjectId of 0
/// is reserved as the null/invalid sentinel.
///
/// @note ObjectIds are the primary key in the Object Store and are used
/// as foreign keys in events, predictions, and recommendations.
enum class ObjectId : uint64_t {
    kNull = 0,
};

/// @brief Identifier for a memory tier node in the Memory Graph.
///
/// TierIds are assigned during topology discovery and remain stable for
/// the lifetime of the engine. Values 0–255 are reserved for well-known
/// tier classes (see TierClass). Values 256+ are assigned dynamically.
enum class TierId : uint16_t {
    kInvalid = 0xFFFF,
};

/// @brief Identifier for a physical or logical device.
///
/// DeviceIds map to physical hardware (GPU 0, NVMe 1, etc.) and are
/// used to correlate tier nodes with their underlying devices.
enum class DeviceId : uint16_t {
    kInvalid = 0xFFFF,
};

/// @brief Identifier for an edge in the Memory Graph.
///
/// EdgeIds uniquely identify a directed connection between two tiers
/// (e.g., VRAM → Pinned RAM via PCIe).
enum class EdgeId : uint32_t {
    kInvalid = 0xFFFFFFFF,
};

/// @brief Identifier for a registered plugin (probe, adapter, sink, model).
enum class PluginId : uint16_t {
    kInvalid = 0xFFFF,
};

/// @brief Identifier for a registered framework adapter.
using AdapterId = PluginId;

/// @brief Identifier for a logical object group.
///
/// Groups allow associating related objects (e.g., all tensors in a layer,
/// all KV cache blocks for a sequence). Groups are created by adapters.
enum class GroupId : uint64_t {
    kNull = 0,
};

// ──────────────────────────────────────────────────────────────────────
// Time Types
// ──────────────────────────────────────────────────────────────────────

/// @brief Monotonic nanosecond timestamp.
///
/// All UME timestamps are monotonic nanoseconds from an unspecified epoch
/// (typically system boot). They are NOT wall-clock times. Use the platform
/// clock (ume::platform::monotonic_now_ns()) to generate timestamps.
///
/// @note A uint64_t nanosecond counter overflows after ~584 years.
enum class Timestamp : uint64_t {
    kZero = 0,
    kMax = std::numeric_limits<uint64_t>::max(),
};

// ──────────────────────────────────────────────────────────────────────
// Interned Strings
// ──────────────────────────────────────────────────────────────────────

/// @brief Handle to an interned string in the global string interner.
///
/// Interned strings are stored once in a contiguous arena. Handles are
/// 32-bit offsets into the arena. Comparison is O(1) integer equality.
/// The null handle (offset 0) maps to the empty string.
enum class InternedString : uint32_t {
    kNull = 0,
};

// ──────────────────────────────────────────────────────────────────────
// Enumerations
// ──────────────────────────────────────────────────────────────────────

/// @brief Classification of memory tier hardware.
///
/// Tier classes define the fundamental characteristics of a memory tier.
/// The Memory Graph uses these to determine default latency/bandwidth
/// estimates before live profiling refines them.
enum class TierClass : uint8_t {
    kVram = 0,          ///< GPU video memory (HBM, GDDR)
    kRam = 1,           ///< System DRAM
    kPinnedRam = 2,     ///< Page-locked (pinned) host memory for DMA
    kCompressedRam = 3, ///< Compressed tier (logical, backed by RAM)
    kNvme = 4,          ///< NVMe solid-state storage
    kRemote = 5,        ///< Remote memory (RDMA, CXL pooled)
    kUnified = 6,       ///< Unified memory (e.g., Apple Silicon)
    kCxl = 7,           ///< CXL-attached memory (CXL.mem)
};

/// @brief Memory access type observed or predicted.
enum class AccessType : uint8_t {
    kRead = 0,
    kWrite = 1,
    kReadWrite = 2,
};

/// @brief Compression codec used for compressed memory tiers.
enum class CompressionCodec : uint8_t {
    kNone = 0,
    kLz4 = 1,
    kZstd = 2,
    kSnappy = 3,
};

// ──────────────────────────────────────────────────────────────────────
// Arithmetic Operators for Strong Types
// ──────────────────────────────────────────────────────────────────────

[[nodiscard]] constexpr uint64_t to_raw(ObjectId id) noexcept {
    return static_cast<uint64_t>(id);
}

[[nodiscard]] constexpr uint16_t to_raw(TierId id) noexcept {
    return static_cast<uint16_t>(id);
}

[[nodiscard]] constexpr uint16_t to_raw(DeviceId id) noexcept {
    return static_cast<uint16_t>(id);
}

[[nodiscard]] constexpr uint32_t to_raw(EdgeId id) noexcept {
    return static_cast<uint32_t>(id);
}

[[nodiscard]] constexpr uint64_t to_raw(Timestamp ts) noexcept {
    return static_cast<uint64_t>(ts);
}

[[nodiscard]] constexpr uint32_t to_raw(InternedString s) noexcept {
    return static_cast<uint32_t>(s);
}

} // namespace ume

// ──────────────────────────────────────────────────────────────────────
// std::hash Specializations
// ──────────────────────────────────────────────────────────────────────

template <>
struct std::hash<ume::ObjectId> {
    [[nodiscard]] size_t operator()(ume::ObjectId id) const noexcept {
        return std::hash<uint64_t>{}(ume::to_raw(id));
    }
};

template <>
struct std::hash<ume::TierId> {
    [[nodiscard]] size_t operator()(ume::TierId id) const noexcept {
        return std::hash<uint16_t>{}(ume::to_raw(id));
    }
};

template <>
struct std::hash<ume::EdgeId> {
    [[nodiscard]] size_t operator()(ume::EdgeId id) const noexcept {
        return std::hash<uint32_t>{}(ume::to_raw(id));
    }
};
