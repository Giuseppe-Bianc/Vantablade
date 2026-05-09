# Tasks: High-Performance Vulkan CPU Memory Allocator Framework

**Input**: Design documents from `/specs/001-vulkan-allocator/`
**Prerequisites**: `plan.md` (required), `spec.md` (required for user stories), `research.md`, `data-model.md`, `contracts/`, `quickstart.md`

**Tests**: Included because each user story defines an independent test scenario and the feature specification expects validation for every story slice.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Wire the build system for the allocator subsystem and allocate a dedicated test target for allocator-specific coverage.

- [ ] T001 [P] Update `src/vantablade_lib/CMakeLists.txt` to compile the allocator subsystem sources `AllocatorMetadata.cpp`, `AllocatorStrategy.cpp`, `RecursionGuard.cpp`, `PlatformMemory.cpp`, `AllocatorTelemetry.cpp`, `PmrResourceAdapter.cpp`, and the existing `VulkanAllocator.cpp` into `vantablade_lib`.
- [ ] T002 [P] Update `test/CMakeLists.txt` to add a dedicated `allocator_tests` executable linked against `Vantablade::vantablade_lib`, `Vantablade::Vantablade_options`, `Vantablade::Vantablade_warnings`, `Catch2::Catch2WithMain`, and `Vulkan::Vulkan`, and register the allocator story test sources for Catch2 discovery.

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Establish the shared allocator value types and internal helper modules that every user story builds on.

**Checkpoint**: The allocator domain types, internal metadata helpers, strategy routing helpers, recursion guard, and platform memory helpers exist and can be referenced by the user-story phases.

- [ ] T003 [P] Create `include/Vantablade/AllocationPolicy.hpp` with the `AllocationPolicy` value object for small, medium, lock-free, and over-allocation thresholds plus cache-line and SIMD alignment fields and invariant validation helpers.
- [ ] T004 [P] Create `include/Vantablade/DebugPolicy.hpp` with the per-instance debug policy flags for leak detection, double-free detection, guard regions, fail-fast behavior, and runtime default override hooks.
- [ ] T005 [P] Create `include/Vantablade/FailurePolicy.hpp` with the failure-mode enum, diagnostic handler signature, recoverable null-return contract, and explicit termination policy fields.
- [ ] T006 [P] Create `include/Vantablade/AllocationStatistics.hpp` with the immutable snapshot schema, per-scope breakdown type, fragmentation counters, peak bytes, and recursion-depth metrics.
- [ ] T007 [P] Create `src/vantablade_lib/AllocatorMetadata.cpp` with the in-band allocation header layout, metadata record lookup helpers, and lifetime bookkeeping primitives.
- [ ] T008 [P] Create `src/vantablade_lib/AllocatorStrategy.cpp` with deterministic slab, region, large, and linear-arena routing for the clarified size boundaries and fast-path threshold.
- [ ] T009 [P] Create `src/vantablade_lib/RecursionGuard.cpp` with thread-local recursion depth tracking, reserve-pool accounting, and max-depth telemetry helpers.
- [ ] T010 [P] Create `src/vantablade_lib/PlatformMemory.cpp` with cross-platform aligned virtual memory reservation, commitment, and release helpers for Windows, Linux, and macOS.

---

## Phase 3: User Story 1 - Initialize Allocator with Custom Policies (Priority: P1) MVP

**Goal**: Instantiate allocator instances with explicit policy objects and observe the configured policy state through snapshot telemetry without creating any Vulkan instance.

**Independent Test**: Construct allocators with default and custom policy objects, query `snapshot()`, and verify the resulting policy values, lifecycle state, and policy-sensitive counters match the inputs exactly.

### Tests for User Story 1

- [ ] T011 [US1] Create `test/allocator_policy_tests.cpp` with Catch2 coverage for allocator construction using default policies, custom size-class policies, lock-free threshold overrides, and telemetry-visible policy reflection.

### Implementation for User Story 1

- [ ] T012 [US1] Expand `include/Vantablade/VulkanAllocator.hpp` with the policy-bearing constructor, `snapshot()`, `set_label()`, `configure_debug()`, `configure_failure()`, and the allocator state and query accessors needed for policy initialization.
- [ ] T013 [US1] Implement policy storage, validation, defaulting, and snapshot reflection in `src/vantablade_lib/VulkanAllocator.cpp` so constructed allocators immediately report the configured policy state.

**Checkpoint**: User Story 1 is complete when policy-configured allocators can be instantiated directly and their policy state is visible through telemetry without any Vulkan integration.

---

## Phase 4: User Story 2 - Register VkAllocationCallbacks with Vulkan Objects (Priority: P1)

**Goal**: Build a fully populated `VkAllocationCallbacks` table and ensure Vulkan instance and device creation route through it with strict alignment and scope preservation.

**Independent Test**: Create and destroy a Vulkan instance and logical device with the allocator table installed, then assert callback invocation counts, scope preservation, pointer round-trips, and alignment correctness under validation layers.

### Tests for User Story 2

- [ ] T014 [US2] Create `test/allocator_callbacks_tests.cpp` with Catch2 coverage for Vulkan instance creation, device creation, command-pool creation, callback invocation tracking, and free-path pointer round-trip checks.

### Implementation for User Story 2

- [ ] T015 [US2] Expand `include/Vantablade/VulkanAllocator.hpp` with the full `VkAllocationCallbacks` API surface, including internal allocation callback declarations and a stable accessor for the callback table.
- [ ] T016 [US2] Implement `pfnAllocation`, `pfnReallocation`, `pfnFree`, `pfnInternalAllocation`, and `pfnInternalFree` in `src/vantablade_lib/VulkanAllocator.cpp` with strict alignment, size-overlap copy semantics, and exact scope propagation.

**Checkpoint**: User Story 2 is complete when Vulkan object creation and destruction can run with the custom callback table and the callbacks report the expected sizes, alignments, and scopes.

---

## Phase 5: User Story 3 - Safe Multithreaded Allocation and Deallocation (Priority: P1)

**Goal**: Ensure concurrent allocate, reallocate, and free operations are race-free while keeping the fast path lock-free up to the clarified threshold.

**Independent Test**: Run 8-thread randomized allocate/free/reallocate churn under ThreadSanitizer, verify zero data races, no deadlocks, and stable active-allocation counters, and confirm producer-consumer handoff works without external locks.

### Tests for User Story 3

- [ ] T017 [US3] Create `test/allocator_concurrency_tests.cpp` with Catch2 coverage for 8-thread random churn, burst allocation/deallocation, and producer-consumer ownership transfer scenarios.

### Implementation for User Story 3

- [ ] T018 [US3] Implement lock-free hot-path allocation and deallocation bookkeeping in `src/vantablade_lib/VulkanAllocator.cpp` using atomics and the bounded slow-path synchronization contract.
- [ ] T019 [US3] Implement reserve-pool fallback and TLS recursion-depth accounting in `src/vantablade_lib/RecursionGuard.cpp` to protect recursive callback entry paths.

**Checkpoint**: User Story 3 is complete when multithreaded allocation and free workloads remain stable under TSAN and the hot path does not require user-provided synchronization.

---

## Phase 6: User Story 4 - Inspect Allocation Telemetry (Priority: P2)

**Goal**: Expose immutable telemetry snapshots with accurate counts, bytes, peak usage, fragmentation estimates, and per-scope distribution data.

**Independent Test**: Known allocation sequences must produce exact totals, monotonic peak bytes, consistent fragmentation metrics, and accurate per-scope distributions while staying under the snapshot latency budget.

### Tests for User Story 4

- [ ] T020 [US4] Create `test/allocator_telemetry_tests.cpp` with Catch2 coverage for exact allocation counts, byte totals, peak usage, fragmentation estimates, scope breakdowns, and snapshot consistency under known workloads.

### Implementation for User Story 4

- [ ] T021 [US4] Extend `include/Vantablade/AllocationStatistics.hpp` with the final immutable snapshot fields, per-scope aggregation semantics, and recursion-depth diagnostic counters required by `snapshot()`.
- [ ] T022 [US4] Implement lightweight counter accumulation and point-in-time snapshot assembly in `src/vantablade_lib/AllocatorTelemetry.cpp`.
- [ ] T023 [US4] Wire the hot allocation and deallocation paths in `src/vantablade_lib/VulkanAllocator.cpp` to update the telemetry counters without introducing global synchronization.

**Checkpoint**: User Story 4 is complete when telemetry snapshots are accurate, immutable, and cheap enough to satisfy the clarified overhead budget.

---

## Phase 7: User Story 5 - Configure Debug Features Independently (Priority: P2)

**Goal**: Toggle leak detection, double-free detection, guard regions, and fail-fast handling per allocator instance without forcing a rebuild.

**Independent Test**: Leak, double-free, and guard-corruption scenarios are detected when enabled, while disabled-debug builds show no hot-path instrumentation overhead.

### Tests for User Story 5

- [ ] T024 [US5] Create `test/allocator_debug_tests.cpp` with Catch2 coverage for leak shutdown reporting, double-free rejection, guard corruption detection, and disabled-debug no-op behavior.

### Implementation for User Story 5

- [ ] T025 [US5] Extend `include/Vantablade/DebugPolicy.hpp` with the runtime policy surface for leak detection, double-free checking, guard regions, fail-fast behavior, and optional compile-time defaults.
- [ ] T026 [US5] Implement allocation record tracking, leak bookkeeping, and double-free detection in `src/vantablade_lib/AllocatorMetadata.cpp`.
- [ ] T027 [US5] Implement guard-region validation and integrity diagnostics in `src/vantablade_lib/VulkanAllocator.cpp`, including fail-fast routing through the configured diagnostic handler.

**Checkpoint**: User Story 5 is complete when each debug feature can be enabled or disabled independently and integrity failures produce deterministic diagnostics.

---

## Phase 8: User Story 6 - Integrate with std::pmr Containers (Priority: P3)

**Goal**: Bridge the allocator into `std::pmr` so standard containers can allocate through the Vulkan allocator without losing metadata, alignment, or telemetry visibility.

**Independent Test**: PMR-backed vectors and nested containers allocate, reallocate, and free through the adapter while telemetry remains visible and alignment is preserved.

### Tests for User Story 6

- [ ] T028 [US6] Create `test/allocator_pmr_tests.cpp` with Catch2 coverage for PMR-backed vector growth, nested container reallocation, aligned allocations, and telemetry visibility.

### Implementation for User Story 6

- [ ] T029 [US6] Create `include/Vantablade/PmrResourceAdapter.hpp` with the concrete `std::pmr::memory_resource` adapter API and upstream allocator ownership rules.
- [ ] T030 [US6] Implement the `std::pmr::memory_resource` bridge in `src/vantablade_lib/PmrResourceAdapter.cpp` so `do_allocate`, `do_deallocate`, and `do_is_equal` delegate to `vnd::VulkanAllocator`.

**Checkpoint**: User Story 6 is complete when standard PMR containers can use the allocator seamlessly and the adapter preserves allocator telemetry and alignment guarantees.

---

## Phase 9: Polish & Cross-Cutting Concerns

**Purpose**: Non-functional cleanup, documentation alignment, and final validation polish.

- [ ] T031 [P] Update `specs/001-vulkan-allocator/quickstart.md` with the final build, validation-layer, sanitizer, and CTest instructions for the allocator feature.
- [ ] T032 [P] Update `specs/001-vulkan-allocator/contracts/public-api.md` so the documented public API, failure semantics, and PMR contract match the implemented surface.
- [ ] T033 Refine `src/vantablade_lib/VulkanAllocator.cpp` with clearer alignment, recursion, shutdown, and diagnostic messages without changing behavior.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No feature dependencies, but it must complete before the allocator source and test files are added.
- **Foundational (Phase 2)**: Depends on Setup completion and blocks all user-story phases.
- **User Stories (Phase 3+)**: Each story depends on the foundation and then proceeds in the spec priority order `P1 -> P2 -> P3`.
- **Polish (Final Phase)**: Depends on completion of the user stories that are being shipped.

### User Story Dependencies

- **User Story 1 (P1)**: First executable slice after the foundation; no dependency on Vulkan integration.
- **User Story 2 (P1)**: Depends on the allocator core from US1 and the shared callback contract, but not on US3 or later stories.
- **User Story 3 (P1)**: Depends on the allocator core from US1 and the shared recursion/metadata helpers, but not on US2.
- **User Story 4 (P2)**: Depends on the allocator core and the shared telemetry types; it can be implemented after the P1 slices are stable.
- **User Story 5 (P2)**: Depends on the allocator core plus the shared metadata and failure-policy hooks.
- **User Story 6 (P3)**: Depends on the allocator core and callback semantics already established by the P1 slices.

### Within Each User Story

- Tests are written before implementation and should fail before the feature code lands.
- Public headers are updated before the corresponding source files rely on the new surface.
- Source files that share a single implementation file are intentionally kept sequential.
- Story completion is checked before moving to the next priority tier.

### Parallel Opportunities

- Setup: `T001` and `T002` can run in parallel because they touch different CMake files.
- Foundation: `T003` through `T006` can run in parallel, and `T007` through `T010` can also be split across implementers once the shared policy types exist.
- Later phases deliberately keep the test file first and then serialize the shared allocator source changes because the same implementation files are extended in multiple stories.

---

## Parallel Example: User Story 1

```bash
# Write the policy regression tests first:
Task: "Create `test/allocator_policy_tests.cpp` with Catch2 coverage for allocator construction using default policies, custom size-class policies, lock-free threshold overrides, and telemetry-visible policy reflection."

# Then land the public API and implementation changes in sequence:
Task: "Expand `include/Vantablade/VulkanAllocator.hpp` with the policy-bearing constructor, `snapshot()`, `set_label()`, `configure_debug()`, `configure_failure()`, and the allocator state and query accessors needed for policy initialization."
Task: "Implement policy storage, validation, defaulting, and snapshot reflection in `src/vantablade_lib/VulkanAllocator.cpp` so constructed allocators immediately report the configured policy state."
```

## Parallel Example: User Story 2

```bash
# First, lock down the Vulkan callback expectations:
Task: "Create `test/allocator_callbacks_tests.cpp` with Catch2 coverage for Vulkan instance creation, device creation, command-pool creation, callback invocation tracking, and free-path pointer round-trip checks."

# Then implement the callback table and callback bodies:
Task: "Expand `include/Vantablade/VulkanAllocator.hpp` with the full `VkAllocationCallbacks` API surface, including internal allocation callback declarations and a stable accessor for the callback table."
Task: "Implement `pfnAllocation`, `pfnReallocation`, `pfnFree`, `pfnInternalAllocation`, and `pfnInternalFree` in `src/vantablade_lib/VulkanAllocator.cpp` with strict alignment, size-overlap copy semantics, and exact scope propagation."
```

## Parallel Example: User Story 3

```bash
# Capture the stress scenarios first:
Task: "Create `test/allocator_concurrency_tests.cpp` with Catch2 coverage for 8-thread random churn, burst allocation/deallocation, and producer-consumer ownership transfer scenarios."

# Then harden the shared implementation paths:
Task: "Implement lock-free hot-path allocation and deallocation bookkeeping in `src/vantablade_lib/VulkanAllocator.cpp` using atomics and the bounded slow-path synchronization contract."
Task: "Implement reserve-pool fallback and TLS recursion-depth accounting in `src/vantablade_lib/RecursionGuard.cpp` to protect recursive callback entry paths."
```

## Parallel Example: User Story 4

```bash
# Define the exact telemetry assertions first:
Task: "Create `test/allocator_telemetry_tests.cpp` with Catch2 coverage for exact allocation counts, byte totals, peak usage, fragmentation estimates, scope breakdowns, and snapshot consistency under known workloads."

# Then extend the snapshot data type and backing implementation:
Task: "Extend `include/Vantablade/AllocationStatistics.hpp` with the final immutable snapshot fields, per-scope aggregation semantics, and recursion-depth diagnostic counters required by `snapshot()`."
Task: "Implement lightweight counter accumulation and point-in-time snapshot assembly in `src/vantablade_lib/AllocatorTelemetry.cpp`."
Task: "Wire the hot allocation and deallocation paths in `src/vantablade_lib/VulkanAllocator.cpp` to update the telemetry counters without introducing global synchronization."
```

## Parallel Example: User Story 5

```bash
# Encode the debug failure cases first:
Task: "Create `test/allocator_debug_tests.cpp` with Catch2 coverage for leak shutdown reporting, double-free rejection, guard corruption detection, and disabled-debug no-op behavior."

# Then implement the debug policy and metadata hooks:
Task: "Extend `include/Vantablade/DebugPolicy.hpp` with the runtime policy surface for leak detection, double-free checking, guard regions, fail-fast behavior, and optional compile-time defaults."
Task: "Implement allocation record tracking, leak bookkeeping, and double-free detection in `src/vantablade_lib/AllocatorMetadata.cpp`."
Task: "Implement guard-region validation and integrity diagnostics in `src/vantablade_lib/VulkanAllocator.cpp`, including fail-fast routing through the configured diagnostic handler."
```

## Parallel Example: User Story 6

```bash
# Lock down the PMR contract before the adapter is written:
Task: "Create `test/allocator_pmr_tests.cpp` with Catch2 coverage for PMR-backed vector growth, nested container reallocation, aligned allocations, and telemetry visibility."

# Then implement the adapter in the public header and source file:
Task: "Create `include/Vantablade/PmrResourceAdapter.hpp` with the concrete `std::pmr::memory_resource` adapter API and upstream allocator ownership rules."
Task: "Implement the `std::pmr::memory_resource` bridge in `src/vantablade_lib/PmrResourceAdapter.cpp` so `do_allocate`, `do_deallocate`, and `do_is_equal` delegate to `vnd::VulkanAllocator`."
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup.
2. Complete Phase 2: Foundational.
3. Complete Phase 3: User Story 1.
4. Stop and validate the allocator policy slice independently by running the allocator policy tests.
5. Only after the policy slice is stable, continue to Vulkan callback integration.

### Incremental Delivery

1. Setup the build and test wiring.
2. Land the shared allocator types and internal helpers.
3. Deliver policy-configurable allocator construction and snapshot reflection.
4. Add Vulkan callback registration and verify callback semantics.
5. Add concurrency hardening and TSAN-backed stress coverage.
6. Add telemetry snapshots, then debug controls, then PMR interoperability.
7. Finish with documentation and diagnostic polish.

### Parallel Team Strategy

1. One developer can work on the allocator CMake wiring while another prepares the allocator-specific test target.
2. The shared policy and helper headers in Phase 2 can be split across multiple implementers.
3. After the foundation is complete, the remaining story phases should be scheduled in priority order because they extend shared allocator state in the same implementation files.
