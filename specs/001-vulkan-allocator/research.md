# Research - Vulkan CPU Memory Allocator Framework

## Scope
Feature: `001-vulkan-allocator`  
Baseline: Vulkan `1.4.341.1`, C++23  
Sources of authority:
- Vulkan Specification and Khronos guidance
- Vulkan validation layers best-practice feedback
- ISO C++23 language model, cppreference, C++ Core Guidelines

## Decision 1 - Strict Vulkan callback semantic compliance
- Decision: Implement all callbacks in `VkAllocationCallbacks`: `pfnAllocation`, `pfnReallocation`, `pfnFree`, `pfnInternalAllocation`, `pfnInternalFree`, preserving `VkSystemAllocationScope` exactly as received.
- Rationale: Ensures correctness and compatibility with loader/driver expectations across platforms and vendors.
- Alternatives considered: Partial callback implementation with `nullptr` internal callbacks.
- Notes: Internal callbacks can be no-op only if tracking policy explicitly disables them, but function pointers remain valid and documented.

## Decision 2 - Deterministic alignment model with hard-fail on integrity violations
- Decision: Use a metadata-header-before-user-pointer layout with mathematically verified alignment; treat misalignment and metadata corruption as unrecoverable integrity faults.
- Rationale: Vulkan allocation callbacks must always honor requested alignment; silent fallback is unsafe.
- Alternatives considered: External pointer-to-metadata maps.
- Notes: Header-in-band design reduces lookup overhead and keeps deallocation deterministic.

## Decision 3 - Recursive callback protection via TLS depth and reserve pools
- Decision: Track recursion depth with thread-local counters and serve recursive allocations from pre-allocated reserve pools.
- Rationale: Prevents allocator self-recursion failures during metadata/diagnostic paths.
- Alternatives considered: Unbounded recursion or global lock around all callback paths.
- Notes: Record max recursion depth in telemetry for diagnostics.

## Decision 4 - Two-tier telemetry model
- Decision: Keep always-on lightweight counters for hot-path metrics and optional heavyweight snapshots for richer analysis.
- Rationale: Meets low overhead goals while preserving observability.
- Alternatives considered: Full telemetry with global synchronization on every allocation.
- Notes: Snapshot API returns immutable `AllocationStatistics` and must stay under the latency constraints from spec.

## Decision 5 - Concurrency model: lock-free fast path <=64 KB, bounded sync for slow paths
- Decision: Use lock-free operations for slab/lower-region allocations up to 64 KB; allow bounded locking for large allocations and cross-region reallocation.
- Rationale: Matches clarified requirements and protects p99 latency targets.
- Alternatives considered: Fully lock-based model or fully lock-free across all paths.
- Notes: Contention metrics are mandatory to verify bounded synchronization behavior.

## Decision 6 - Tiered failure semantics
- Decision: Recoverable failures (`OOM`, address-space exhaustion) return `nullptr`; unrecoverable integrity faults call diagnostic handler then terminate safely.
- Rationale: Aligns Vulkan expectations for alloc failures while preserving fail-fast correctness for corruption.
- Alternatives considered: Always throw exceptions or always continue with degraded behavior.
- Notes: Public API remains explicit about no hidden recovery on integrity violations.

## Decision 7 - Project-appropriate public contract (library API)
- Decision: Produce a C++ library-facing contract in `contracts/public-api.md` instead of REST/CLI schema.
- Rationale: This project exposes a C++ API and Vulkan callback interface, not web/service endpoints.
- Alternatives considered: OpenAPI-like external contract.
- Notes: Contract captures invariants, error semantics, threading guarantees, and performance SLOs.

## Decision 8 - C++23 implementation style constraints
- Decision: Enforce RAII ownership boundaries, strong enums, `noexcept` correctness, atomics from `<atomic>`, and explicit lifetime/synchronization contracts.
- Rationale: Matches constitution and modern C++ safety/maintainability guidance.
- Alternatives considered: Legacy C-style ownership patterns with implicit invariants.
- Notes: Low-level pointer math remains allowed only at Vulkan interop boundaries with audited invariants.

## Decision 9 - Validation-first workflow
- Decision: Treat Vulkan validation-layer issues, sanitizer findings, and stress-test races as blocking defects.
- Rationale: Required for reproducibility and cross-platform correctness.
- Alternatives considered: Best-effort validation limited to debug sessions.
- Notes: Planned checks include validation layers, TSAN/ASAN/UBSAN where supported, and CTest/Catch2 stress suites.

## Decision 10 - Portability-first defaults
- Decision: Keep default behavior portable across Windows/Linux/macOS (MoltenVK path), isolate platform-specific VM primitives behind abstractions.
- Rationale: Avoids vendor/driver coupling and supports long-term maintainability.
- Alternatives considered: Vendor-specific optimization shortcuts in core path.
- Notes: Any platform-specific optimization must be opt-in and benchmarked against portable baseline.

## Additional Resolved Clarifications
- Debug feature configuration: runtime policy per allocator instance, optional CMake defaults, optional environment overrides.
- Strategy boundaries: slab `0-4 KB`, region `4 KB-1 MB`, large `>1 MB`, lock-free fast path threshold `64 KB`.
- Telemetry API: immutable point-in-time snapshot via `snapshot()`.
- Labeling model: flat string labels up to 64 bytes via `set_label(string_view)` path.
- Security scope: async-signal safety and secure-zeroing deferred beyond MVP.

## Validation Checklist for Implementation Phase
- Vulkan callback ABI and semantics verified against Vulkan 1.4.341.1.
- Alignment and metadata invariants unit-tested and stress-tested.
- Thread-safety stress: 8+ threads random alloc/free with race analysis.
- Reallocation overlap semantics validated by deterministic content checks.
- Telemetry overhead and snapshot latency measured against success criteria.
- Cross-platform smoke runs for Windows/Linux/macOS behavior consistency.
