# Quickstart - Vulkan CPU Memory Allocator Framework

## Purpose
Run and validate the allocator feature in a reproducible way aligned with Vulkan 1.4.341.1 and C++23 constraints.

## Prerequisites
- Vulkan SDK `1.4.341.1` installed and visible to CMake.
- C++23-capable compiler toolchain configured by the repository.
- CMake and Ninja/MSBuild as used by this repository.
- Validation layers available (`VK_LAYER_KHRONOS_validation`).

## Build
From repository root:

```powershell
cmake -S . -B build -DCMAKE_CXX_STANDARD=23
cmake --build build --config Debug
```

## Scenario 1 - Initialize allocator with policy (US1)
Expected outcome: policy values are accepted and reflected by snapshot telemetry.

```cpp
vnd::AllocationPolicy policy{};
policy.smallMaxBytes = 4096;
policy.mediumMaxBytes = 1024 * 1024;
policy.lockFreeFastPathMaxBytes = 64 * 1024;
policy.overallocationRatio = 1.5f;

vnd::VulkanAllocator allocator{/*policy, debug, failure*/};
auto stats = allocator.snapshot();
```

Validation checks:
- policy boundaries are valid
- allocator enters active lifecycle state
- telemetry is queryable

## Scenario 2 - Register callbacks with Vulkan (US2)
Expected outcome: Vulkan alloc/free/realloc paths invoke allocator callbacks with scope preserved.

```cpp
vnd::VulkanAllocator allocator{/*...*/};
VkAllocationCallbacks callbacks = allocator.getCallbacks();

VkInstanceCreateInfo ci{ /* ... */ };
VkInstance instance = VK_NULL_HANDLE;
VkResult r = vkCreateInstance(&ci, &callbacks, &instance);
```

Validation checks:
- callback pointers are non-null
- `pUserData` points to allocator instance
- validation layers report no callback contract violations

## Scenario 3 - Multithreaded stress safety (US3)
Expected outcome: no deadlocks/data races, predictable latency behavior.

Test profile:
- 8 threads
- randomized allocation sizes and alignments
- mixed allocate/reallocate/free operations

Validation checks:
- no TSAN races
- no integrity-failure events under valid input
- snapshot counters remain consistent

## Scenario 4 - Telemetry snapshot and diagnostics (US4)
Expected outcome: immutable snapshot reflects allocation state with expected metrics.

Validation checks:
- totals and active counts track known workloads
- peak bytes monotonic
- per-scope distribution populated correctly
- snapshot latency meets target envelope

## Scenario 5 - Debug feature toggles (US5)
Expected outcome: debug behaviors are independently switchable at runtime policy level.

Validation checks:
- leak detection reports intentionally leaked allocation on shutdown
- double-free detection triggers diagnostic handler
- guard region corruption triggers integrity failure path
- disabling debug options removes instrumentation overhead from hot path

## Scenario 6 - PMR adapter interoperability (US6)
Expected outcome: standard PMR containers allocate through allocator while preserving telemetry.

Validation checks:
- pmr container allocations visible in snapshot statistics
- PMR reallocation semantics preserve data
- deallocation path releases records correctly

## Recommended Validation Run

```powershell
ctest --test-dir build --output-on-failure
```

Additionally verify:
- Vulkan validation-layer output is clean for callback registration scenarios.
- Sanitizer-enabled builds pass targeted stress tests.

## Troubleshooting
- If validation layers are not found, verify Vulkan SDK environment configuration.
- If snapshot timing exceeds target, disable heavyweight diagnostics and re-measure.
- If thread-safety tests fail, inspect lock-free threshold routing and metadata ownership invariants.
