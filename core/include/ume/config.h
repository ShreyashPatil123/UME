#pragma once

/// @file config.h
/// @brief Engine configuration schema with TOML deserialization.
///
/// Configuration is loaded from a TOML file (default: ume.toml) and validated
/// at engine startup. Individual settings can be overridden programmatically.

#include "ume/status.h"

#include <cstdint>
#include <string>

namespace ume {

/// @brief Journal configuration.
struct JournalConfig {
    /// Size of each journal segment in bytes. Default: 64 MB.
    uint64_t segment_size_bytes = 64 * 1024 * 1024;

    /// How long to keep hot (in-memory) journal segments in seconds. Default: 60s.
    uint32_t hot_retention_seconds = 60;

    /// How long to keep warm (on-disk) segments before compaction. Default: 3600s.
    uint32_t warm_retention_seconds = 3600;

    /// Whether background compaction is enabled. Default: true.
    bool compaction_enabled = true;
};

/// @brief Object Store configuration.
struct ObjectStoreConfig {
    /// Number of shards for the concurrent hash map. Default: 256.
    /// Must be a power of two.
    uint32_t shard_count = 256;

    /// Objects smaller than this threshold may be coalesced. Default: 4096 bytes.
    uint64_t small_alloc_threshold_bytes = 4096;

    /// Interval between tombstone reaping passes in seconds. Default: 5s.
    uint32_t tombstone_reap_interval_seconds = 5;
};

/// @brief Memory Graph configuration.
struct GraphConfig {
    /// Maximum time to wait for stalled RCU readers in milliseconds. Default: 100ms.
    uint32_t rcu_epoch_timeout_ms = 100;
};

/// @brief Probe (hardware telemetry) configuration.
struct ProbeConfig {
    /// Hardware sampling rate in Hz. Default: 10 Hz.
    uint32_t sample_rate_hz = 10;

    /// Enable CUDA-specific probes (requires CUDA runtime). Default: false.
    bool cuda_enabled = false;

    /// Enable NVML-based GPU monitoring. Default: false.
    bool nvml_enabled = false;
};

/// @brief Framework adapter configuration.
struct AdapterConfig {
    /// Maximum time to hold the Python GIL in microseconds. Default: 200 µs.
    uint32_t python_gil_max_hold_us = 200;

    /// Batch collection frequency for Python adapters in Hz. Default: 5 Hz.
    uint32_t python_batch_hz = 5;
};

/// @brief Dashboard configuration.
struct DashboardConfig {
    /// WebSocket server port. Default: 9100.
    uint16_t websocket_port = 9100;

    /// REST API server port. Default: 9101.
    uint16_t rest_port = 9101;

    /// Maximum concurrent WebSocket clients. Default: 8.
    uint16_t max_clients = 8;
};

/// @brief Prediction engine configuration.
struct PredictionConfig {
    /// Prediction cycle interval in milliseconds. Default: 100ms.
    uint32_t cycle_interval_ms = 100;

    /// Comma-separated list of enabled prediction models. Default: "ewma,lru".
    std::string models_enabled = "ewma,lru";
};

/// @brief Advisor configuration.
struct AdvisorConfig {
    /// Advisory cycle interval in milliseconds. Default: 100ms.
    uint32_t cycle_interval_ms = 100;

    /// Whether the advisor is enabled. Default: true.
    bool enabled = true;
};

/// @brief Engine-level configuration.
struct EngineConfig {
    /// Logging level: "trace", "debug", "info", "warn", "error", "off". Default: "info".
    std::string log_level = "info";

    /// Maximum CPU overhead budget as a percentage of one core. Default: 2.0%.
    float overhead_budget_cpu_percent = 2.0f;

    /// Maximum memory overhead budget in MB. Default: 250 MB.
    uint32_t overhead_budget_memory_mb = 250;
};

/// @brief Complete UME configuration.
///
/// Contains all subsystem configurations. Loaded from TOML at startup.
///
/// Usage:
/// @code
///     auto result = Config::load_from_file("ume.toml");
///     if (result.ok()) {
///         Engine engine(result.value());
///     }
/// @endcode
struct Config {
    EngineConfig engine;
    JournalConfig journal;
    ObjectStoreConfig object_store;
    GraphConfig graph;
    ProbeConfig probes;
    AdapterConfig adapters;
    DashboardConfig dashboard;
    PredictionConfig prediction;
    AdvisorConfig advisor;

    /// @brief Load configuration from a TOML file.
    ///
    /// @param path Path to the TOML configuration file.
    /// @return Config on success, or an error status.
    [[nodiscard]] static Result<Config> load_from_file(const std::string& path);

    /// @brief Returns a configuration suitable for unit tests.
    ///
    /// Uses in-memory journal (1 MB segments), small shard count (16),
    /// and disabled hardware probes.
    [[nodiscard]] static Config default_test_config() noexcept;

    /// @brief Validates all configuration values are within acceptable ranges.
    ///
    /// @return kOk if valid, kInvalidConfig with details otherwise.
    [[nodiscard]] Status validate() const noexcept;
};

} // namespace ume
