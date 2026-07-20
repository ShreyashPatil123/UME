/// @file config.cpp
/// @brief Implementation of UME configuration management.
///
/// Parses TOML configuration files and validates all settings.

#include "ume/config.h"

#include <bit>
#include <fstream>
#include <toml.hpp>
#include <type_traits>

namespace ume {

namespace {

// Helper to safely extract integer types from toml11 (which parses integers as int64_t)
template <typename IntType>
IntType get_int(const toml::value& v, const std::string& key, IntType default_val) {
    if (v.contains(key)) {
        return static_cast<IntType>(
            toml::find_or<std::int64_t>(v, key, static_cast<std::int64_t>(default_val)));
    }
    return default_val;
}

std::string get_str(const toml::value& v, const std::string& key, const std::string& default_val) {
    if (v.contains(key)) {
        return toml::find_or<std::string>(v, key, default_val);
    }
    return default_val;
}

bool get_bool(const toml::value& v, const std::string& key, bool default_val) {
    if (v.contains(key)) {
        return toml::find_or<bool>(v, key, default_val);
    }
    return default_val;
}

float get_float(const toml::value& v, const std::string& key, float default_val) {
    if (v.contains(key)) {
        return static_cast<float>(toml::find_or<double>(v, key, static_cast<double>(default_val)));
    }
    return default_val;
}

} // namespace

Result<Config> Config::load_from_file(const std::string& path) {
    // Check file exists before attempting parse
    {
        std::ifstream test(path);
        if (!test.good()) {
            return Status::kFileNotFound;
        }
    }

    try {
        const auto data = toml::parse(path);
        Config config;

        // [engine]
        if (data.contains("engine")) {
            const auto& sec = toml::find(data, "engine");
            config.engine.log_level = get_str(sec, "log_level", config.engine.log_level);
            config.engine.overhead_budget_cpu_percent = get_float(
                sec, "overhead_budget_cpu_percent", config.engine.overhead_budget_cpu_percent);
            config.engine.overhead_budget_memory_mb =
                get_int(sec, "overhead_budget_memory_mb", config.engine.overhead_budget_memory_mb);
        }

        // [journal]
        if (data.contains("journal")) {
            const auto& sec = toml::find(data, "journal");
            uint64_t default_mb = config.journal.segment_size_bytes / (1024 * 1024);
            config.journal.segment_size_bytes =
                get_int(sec, "segment_size_mb", default_mb) * 1024 * 1024;
            config.journal.hot_retention_seconds =
                get_int(sec, "hot_retention_seconds", config.journal.hot_retention_seconds);
            config.journal.warm_retention_seconds =
                get_int(sec, "warm_retention_seconds", config.journal.warm_retention_seconds);
            config.journal.compaction_enabled =
                get_bool(sec, "compaction_enabled", config.journal.compaction_enabled);
        }

        // [object_store]
        if (data.contains("object_store")) {
            const auto& sec = toml::find(data, "object_store");
            config.object_store.shard_count =
                get_int(sec, "shard_count", config.object_store.shard_count);
            config.object_store.small_alloc_threshold_bytes =
                get_int(sec, "small_alloc_threshold_bytes",
                        config.object_store.small_alloc_threshold_bytes);
            config.object_store.tombstone_reap_interval_seconds =
                get_int(sec, "tombstone_reap_interval_seconds",
                        config.object_store.tombstone_reap_interval_seconds);
        }

        // [graph]
        if (data.contains("graph")) {
            const auto& sec = toml::find(data, "graph");
            config.graph.rcu_epoch_timeout_ms =
                get_int(sec, "rcu_epoch_timeout_ms", config.graph.rcu_epoch_timeout_ms);
        }

        // [probes]
        if (data.contains("probes")) {
            const auto& sec = toml::find(data, "probes");
            config.probes.sample_rate_hz =
                get_int(sec, "sample_rate_hz", config.probes.sample_rate_hz);
            config.probes.cuda_enabled = get_bool(sec, "cuda_enabled", config.probes.cuda_enabled);
            config.probes.nvml_enabled = get_bool(sec, "nvml_enabled", config.probes.nvml_enabled);
        }

        // [adapters]
        if (data.contains("adapters")) {
            const auto& sec = toml::find(data, "adapters");
            config.adapters.python_gil_max_hold_us =
                get_int(sec, "python_gil_max_hold_us", config.adapters.python_gil_max_hold_us);
            config.adapters.python_batch_hz =
                get_int(sec, "python_batch_hz", config.adapters.python_batch_hz);
        }

        // [dashboard]
        if (data.contains("dashboard")) {
            const auto& sec = toml::find(data, "dashboard");
            config.dashboard.websocket_port =
                get_int(sec, "websocket_port", config.dashboard.websocket_port);
            config.dashboard.rest_port = get_int(sec, "rest_port", config.dashboard.rest_port);
            config.dashboard.max_clients =
                get_int(sec, "max_clients", config.dashboard.max_clients);
        }

        // [prediction]
        if (data.contains("prediction")) {
            const auto& sec = toml::find(data, "prediction");
            config.prediction.cycle_interval_ms =
                get_int(sec, "cycle_interval_ms", config.prediction.cycle_interval_ms);
            config.prediction.models_enabled =
                get_str(sec, "models_enabled", config.prediction.models_enabled);
        }

        // [advisor]
        if (data.contains("advisor")) {
            const auto& sec = toml::find(data, "advisor");
            config.advisor.cycle_interval_ms =
                get_int(sec, "cycle_interval_ms", config.advisor.cycle_interval_ms);
            config.advisor.enabled = get_bool(sec, "enabled", config.advisor.enabled);
        }

        // Validate parsed config
        Status validation = config.validate();
        if (is_error(validation)) {
            return validation;
        }

        return config;

    } catch (const toml::syntax_error&) {
        return Status::kInvalidConfig;
    } catch (const std::exception&) {
        return Status::kIoError;
    }
}

Config Config::default_test_config() noexcept {
    Config config;
    config.engine.log_level = "debug";
    config.engine.overhead_budget_cpu_percent = 50.0f; // Relaxed for tests
    config.engine.overhead_budget_memory_mb = 100;
    config.journal.segment_size_bytes = 1 * 1024 * 1024; // 1 MB segments
    config.journal.hot_retention_seconds = 10;
    config.journal.compaction_enabled = false;
    config.object_store.shard_count = 16; // Small for tests
    config.graph.rcu_epoch_timeout_ms = 50;
    config.probes.sample_rate_hz = 1;
    config.probes.cuda_enabled = false;
    config.probes.nvml_enabled = false;
    config.dashboard.websocket_port = 0; // Disabled
    config.dashboard.rest_port = 0;      // Disabled
    config.prediction.cycle_interval_ms = 1000;
    config.advisor.enabled = false;
    return config;
}

Status Config::validate() const noexcept {
    // Engine validation
    if (engine.overhead_budget_cpu_percent < 0.0f || engine.overhead_budget_cpu_percent > 100.0f) {
        return Status::kInvalidConfig;
    }
    if (engine.overhead_budget_memory_mb < 10) {
        return Status::kInvalidConfig;
    }

    // Journal validation
    if (journal.segment_size_bytes < 1024 * 1024) { // Minimum 1 MB
        return Status::kInvalidConfig;
    }

    // Object Store validation: shard_count must be power of two
    if (object_store.shard_count == 0 || !std::has_single_bit(object_store.shard_count)) {
        return Status::kInvalidConfig;
    }

    // Probe validation
    if (probes.sample_rate_hz < 1 || probes.sample_rate_hz > 1000) {
        return Status::kInvalidConfig;
    }

    return Status::kOk;
}

} // namespace ume
