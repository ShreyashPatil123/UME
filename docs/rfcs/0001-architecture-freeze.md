# RFC: v3 Architecture Freeze

**Status:** Accepted
**Author:** UME Core Team
**Date:** 2026-07-20

## Summary

This RFC documents the official architecture freeze for UME v3. The core event-sourced architecture, Memory Object Model (MOM), and UM-IR foundations are now considered stable. Any subsequent modifications to these core components must go through a rigorous change management and RFC process.

## Motivation

As UME transitions into Phase 1A (Observability Foundation), it is critical to have a stable bedrock upon which to build backends, predictors, and simulators. The event-sourced paradigm and the MOM represent a significant departure from traditional memory management, and solidifying these APIs prevents cascading rewrites across the ecosystem.

## Detailed Design

The frozen architecture encompasses:
1.  **Core Event Store:** Immutable append-only log of memory operations.
2.  **Memory Object Model (MOM):** Abstract representation of physical/virtual topologies.
3.  **Memory Graph:** Dependency tracking system.
4.  **UM-IR Core Elements:** The base intermediate representation schema for memory transformations.
5.  **Error Handling Model:** Strict use of Status/Result types across core C++20 boundaries (no exceptions).

## Alternatives Considered

We considered allowing fluid API changes during Phase 1A, but determined it would severely hinder parallel development of the CUDA and HIP backends by the wider community.

## Impact

*   **API Stability:** Core C++20 headers within `ume::core` are now semi-stable.
*   **Process:** Any changes to the aforementioned systems require a new RFC and approval from at least two core maintainers.

## Implementation Plan

The freeze is effective immediately upon acceptance of this RFC. Development will now shift focus to implementing the specific backends and the predictive models on top of these stable interfaces.

## Unresolved Questions

None. The architecture principles are settled.
