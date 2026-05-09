# Implementation Plan: Vulkan CPU Memory Allocator Framework

**Branch**: `001-vulkan-allocator` | **Date**: 2026-05-09 | **Spec**: `specs/001-vulkan-allocator/spec.md`
**Input**: Feature specification from `/specs/001-vulkan-allocator/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

## Summary

Implement a robust CPU-side Vulkan allocation callback framework compliant with Vulkan 1.4.341.1 and modern C++23 practices. The plan delivers: strict `VkAllocationCallbacks` semantic compliance, lock-free fast paths up to 64 KB, deterministic strategy selection across slab/region/large allocators, two-tier telemetry, configurable diagnostics, and portability across Windows/Linux/macOS (including MoltenVK support path). All architecture and validation choices are constrained by Khronos guidance, Vulkan validation-layer feedback, and C++ Core Guidelines.

## Technical Context

**Language/Version**: C++23 (required), Vulkan SDK/API 1.4.341.1  
**Primary Dependencies**: Vulkan loader+headers, validation layers, Catch2 (tests), CTest, existing project libs (`fmt`, `spdlog`, `CLI11`)  
**Storage**: N/A (in-memory metadata and telemetry; optional snapshot history ring buffer)  
**Testing**: CTest + Catch2 unit/constexpr tests, Vulkan validation layers, ThreadSanitizer/AddressSanitizer/UBSan where supported, stress/fuzz test targets  
**Target Platform**: Windows 10+, Linux (glibc 2.29+), macOS 10.15+ (via MoltenVK where applicable)  
**Project Type**: Native C++ rendering library + executable integration (single repository)  
**Performance Goals**: 
- small alloc avg < 500 ns (SC-001)
- lock-free path <= 64 KB with p99 < 1 us @ 8 threads (SC-001a)
- > 1M alloc/dealloc ops/sec aggregate @ 8 threads (SC-003)
- lightweight telemetry overhead < 50 ns/op (SC-005a)
- snapshot query latency < 10 us under contention (FR-045/FR-044b)
**Constraints**: 
- strict Vulkan callback semantics and alignment guarantees (FR-001..FR-008)
- no undefined behavior, no undocumented driver assumptions
- explicit lifetime/ownership/synchronization (constitution principles I-III)
- portability-first defaults over vendor-specific shortcuts
- deterministic failure semantics (recoverable null return, unrecoverable fail-fast with diagnostics)
**Scale/Scope**: MVP framework in existing `vantablade_lib` and `include/Vantablade` surface; includes callback layer, strategy core, telemetry/diagnostics, PMR adapter, validation-focused tests

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Pre-Phase-0 gate status:

- Gate 1 - Vulkan specification alignment: PASS
  - API behavior constrained to Vulkan 1.4.341.1 callback contract and Khronos validation expectations.
- Gate 2 - C++23 baseline: PASS
  - Public/internal interfaces target C++23 idioms (`enum class`, `noexcept`, strong typing, RAII, atomics) with explicit ownership.
- Gate 3 - Lifetime/ownership/synchronization explicitness: PASS
  - Planned modules separate allocation logic, metadata ownership, and synchronization policy.
- Gate 4 - Validation and reproducibility: PASS
  - Validation layers + sanitizers + stress tests are first-class acceptance gates.
- Gate 5 - Measured performance and portability: PASS
  - Quantified latency/throughput/overhead targets mapped to explicit test workloads.

Post-Phase-1 re-check status:

- PASS: Phase-1 artifacts preserve specification-first modeling, project-appropriate contracts, and measurable validation criteria.
- No constitution violations requiring justification in Complexity Tracking.

## Project Structure

### Documentation (this feature)

```text
specs/001-vulkan-allocator/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
include/
└── Vantablade/
    ├── VulkanAllocator.hpp
    ├── Device.hpp
    ├── SwapChain.hpp
    └── ...

src/
├── vantablade/
├── vantablade_lib/
└── vantablade_Core_lib/

test/
├── CMakeLists.txt
├── tests.cpp
└── constexpr_tests.cpp

fuzz_test/
└── CMakeLists.txt

specs/
└── 001-vulkan-allocator/
    ├── spec.md
    ├── plan.md
    ├── research.md
    ├── data-model.md
    ├── quickstart.md
    └── contracts/
```

**Structure Decision**: Use the existing single native C++ project layout. The allocator feature is implemented as a library-centric subsystem under `include/Vantablade` + `src/vantablade_lib`, validated via `test/` and stress/fuzz extensions, with spec artifacts localized to `specs/001-vulkan-allocator`.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| None | N/A | N/A |
