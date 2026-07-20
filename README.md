# UME: Unified Memory Engine

**Universal AI Memory Virtualization Platform**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/ume-project/ume/actions)
[![License: Apache-2.0 OR MIT](https://img.shields.io/badge/License-Apache--2.0_OR_MIT-blue.svg)](https://opensource.org/licenses)
[![Version](https://img.shields.io/badge/version-v3.0.0--alpha-orange.svg)](https://github.com/ume-project/ume/releases)

## Overview

The Unified Memory Engine (UME) is a high-performance, cross-platform memory virtualization platform designed specifically for the rigorous demands of modern AI workloads. By decoupling memory operations from underlying hardware constraints, UME allows AI applications to scale seamlessly across heterogeneous compute environments—including CPUs, diverse multi-vendor GPUs, and specialized accelerators.

UME introduces a revolutionary approach to memory management through its Event-Sourced Architecture and comprehensive Memory Object Model. Instead of traditional static memory allocations, UME tracks every memory state change as a discrete event, enabling unprecedented capabilities in debugging, simulation, state prediction, and dynamic optimization.

Whether you are developing large language models, complex neural networks, or high-throughput data processing pipelines, UME provides the observability, control, and performance necessary to maximize hardware utilization and accelerate development cycles.

## Key Features

*   **Event-Sourced Architecture:** All memory operations are recorded as immutable events, allowing for time-travel debugging, deterministic replay, and deep observability.
*   **Memory Object Model (MOM):** An abstract representation of memory topologies, enabling portable and robust memory management across diverse hardware.
*   **Memory Graph:** A dynamic, directed acyclic graph representing dependencies and relationships between memory allocations and operations.
*   **UM-IR (Unified Memory Intermediate Representation):** A specialized intermediate representation for memory operations, enabling advanced static and dynamic analysis and optimization.
*   **Prediction Engine:** Uses historical event data to predict future memory access patterns, enabling proactive prefetching and intelligent eviction strategies.
*   **Advisor System:** Analyzes memory usage patterns and provides actionable recommendations to optimize performance and reduce fragmentation.
*   **Simulator:** A robust simulation environment to test memory strategies and optimizations without requiring physical hardware.
*   **Cross-Platform & Multi-Vendor GPU Support:** Natively supports Windows, Linux, and macOS, with deep integration for NVIDIA (CUDA), AMD (HIP/ROCm), and Intel (SYCL) hardware.

## Architecture

UME's architecture is built around a core `ume::core` engine that orchestrates interactions between various subsystems, including the `EventStore`, `MemoryGraph`, and specialized backends (e.g., CUDA, HIP). The event stream serves as the single source of truth, feeding the `PredictionEngine` and `Advisor` for continuous optimization.

For a comprehensive dive into the design and component interactions, please see the [Architecture Documentation](docs/architecture/).

## Quick Start

### Prerequisites

*   **CMake:** >= 3.25
*   **Compiler:** C++20 compatible compiler (GCC 11+, Clang 14+, MSVC 19.30+)
*   **GPU Toolkit (Optional but recommended):** CUDA Toolkit, ROCm, or Intel oneAPI

### Building UME

```bash
# Clone the repository
git clone https://github.com/ume-project/ume.git
cd ume

# Configure the build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build . --config Release -j $(nproc)
```

### Running an Example

After building, you can run one of the provided examples to see UME in action:

```bash
./bin/ume_example_basic_allocation
```

## Project Status

UME is currently in **Phase 1A: Observability Foundation**. Active development is focused on solidifying the Event-Sourced Architecture, implementing the core Memory Object Model, and establishing the base UM-IR.

## License

UME is dual-licensed under either the Apache License, Version 2.0 or the MIT License, at your option. See the [LICENSE-APACHE](LICENSE-APACHE) and [LICENSE-MIT](LICENSE-MIT) files for specific terms.

## Contributing

We welcome contributions from the community! Please read our [Contributing Guide](docs/contributor/CONTRIBUTING.md) to learn about our development process, how to propose bugfixes and improvements, and how to build and test your changes. Ensure you are familiar with our [Code of Conduct](docs/contributor/CODE_OF_CONDUCT.md).
