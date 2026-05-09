# Data Model - Vulkan CPU Memory Allocator Framework

## Modeling Rule
Every entity and field below is traced to the feature spec requirements and clarifications. No speculative entities are included.

## Entity: AllocatorInstance
Traceability:
- User Stories: US1, US2, US3, US4, US5
- Requirements: FR-001..FR-019, FR-035a..FR-035c, FR-074..FR-080, FR-089..FR-109

Fields:
- `id: uint64_t`
- `callbacks: VkAllocationCallbacks`
- `policy: AllocationPolicy`
- `debugPolicy: DebugPolicy`
- `failurePolicy: FailurePolicy`
- `telemetryConfig: TelemetryConfig`
- `lifecycleState: AllocatorLifecycleState`

Validation Rules:
- `callbacks.pUserData` must reference this instance.
- All callback function pointers must be non-null and ABI-compatible.
- `lifecycleState` transitions must be monotonic (`Uninitialized -> Active -> ShuttingDown -> Destroyed`).

State Transitions:
- `Uninitialized -> Active` on successful construction/init.
- `Active -> ShuttingDown` on shutdown begin.
- `ShuttingDown -> Destroyed` after resources and trackers are released.

## Entity: AllocationPolicy
Traceability:
- User Stories: US1
- Requirements: FR-014..FR-019, FR-068..FR-073

Fields:
- `smallMaxBytes: size_t` (default 4096)
- `mediumMaxBytes: size_t` (default 1048576)
- `lockFreeFastPathMaxBytes: size_t` (default 65536)
- `overallocationRatio: float` (allowed values 1.2, 1.5, 2.0)
- `cacheLineAlignment: size_t`
- `simdAlignment: size_t`

Validation Rules:
- `smallMaxBytes < mediumMaxBytes`.
- `lockFreeFastPathMaxBytes <= mediumMaxBytes`.
- `overallocationRatio` constrained to guidance set unless explicit override policy is documented.

## Entity: AllocationRequest
Traceability:
- User Stories: US2, US3
- Requirements: FR-001, FR-002, FR-007, FR-020..FR-024

Fields:
- `sizeBytes: size_t`
- `alignmentBytes: size_t`
- `scope: VkSystemAllocationScope`
- `operation: AllocationOperation` (`Allocate`, `Reallocate`, `Free`)
- `label: string_view` (optional, max 64 bytes)

Validation Rules:
- `alignmentBytes` must be non-zero and power-of-two.
- `scope` must map to known Vulkan scope enum values.
- For `Reallocate`, `originalPointer` may be null only when semantically equivalent to allocate.

## Entity: AllocationRecord
Traceability:
- User Stories: US4, US5
- Requirements: FR-025..FR-034, FR-054..FR-059a

Fields:
- `userPointer: void*`
- `basePointer: void*`
- `sizeBytes: size_t`
- `alignmentBytes: size_t`
- `scope: VkSystemAllocationScope`
- `label[64]: char`
- `originFile: string` (optional)
- `originLine: uint32_t` (optional)
- `originFunction: string` (optional)
- `allocationTimestamp: uint64_t`
- `guardPrefixValid: bool`
- `guardSuffixValid: bool`
- `isFreed: bool`

Validation Rules:
- `userPointer` uniqueness among active records.
- `basePointer` must be non-null for active records.
- `sizeBytes > 0` for active records.

## Entity: RecursionGuardState
Traceability:
- Edge Case clarification + FR-008a..FR-008c

Fields:
- `tlsDepth: uint32_t`
- `maxObservedDepth: atomic<uint32_t>`
- `reservePoolAvailableBytes: size_t`

Validation Rules:
- `tlsDepth` increments/decrements must be balanced per thread.
- Reserve pool consumption may not exceed configured capacity.

## Entity: AllocationStatistics
Traceability:
- User Story: US4
- Requirements: FR-036..FR-045

Fields:
- `totalAllocationCount: uint64_t`
- `totalAllocatedBytes: uint64_t`
- `totalDeallocatedBytes: uint64_t`
- `activeAllocationCount: uint64_t`
- `activeAllocationBytes: uint64_t`
- `peakAllocationBytes: uint64_t`
- `internalFragmentationPercent: float`
- `externalFragmentationPercent: float`
- `scopeBreakdown: map<VkSystemAllocationScope, ScopeStatistics>`
- `maxRecursionDepth: uint32_t`

Validation Rules:
- Counters monotonic where applicable (`total*` non-decreasing).
- `activeAllocationBytes <= peakAllocationBytes` always.
- Snapshot must be internally consistent at collection point.

## Entity: DebugPolicy
Traceability:
- User Story: US5
- Requirements: FR-026..FR-035c

Fields:
- `enableLeakDetection: bool`
- `enableDoubleFreeDetection: bool`
- `enableGuardRegions: bool`
- `failFastOnIntegrityViolation: bool`

Validation Rules:
- Guard-region checks only active when enabled.
- Fail-fast behavior must route through diagnostic handler before termination.

## Entity: FailurePolicy
Traceability:
- Clarifications + FR-046..FR-053

Fields:
- `diagnosticHandler: function<void(DiagnosticEvent)>`
- `recoverableReturnNull: bool`
- `terminateOnIntegrityViolation: bool`

Validation Rules:
- Recoverable allocation failures return null without metadata corruption.
- Integrity violations must produce explicit diagnostic event.

## Entity: PmrResourceAdapter
Traceability:
- User Story: US6
- Requirements: FR-081..FR-088

Fields:
- `upstreamAllocator: AllocatorInstance*`
- `propagateStatistics: bool`
- `propagateLabels: bool`

Validation Rules:
- Adapter must preserve requested alignment in PMR calls.
- PMR deallocation must map to the same allocator ownership domain.

## Relationships
- `AllocatorInstance 1 -> 1 AllocationPolicy`
- `AllocatorInstance 1 -> 1 DebugPolicy`
- `AllocatorInstance 1 -> 1 FailurePolicy`
- `AllocatorInstance 1 -> N AllocationRecord`
- `AllocatorInstance 1 -> 1 RecursionGuardState`
- `AllocatorInstance 1 -> N AllocationStatistics` (snapshots over time)
- `PmrResourceAdapter N -> 1 AllocatorInstance`

## Reverse Traceability Check
- All major requirement clusters are represented: callback semantics, synchronization, strategy framework, diagnostics, telemetry, failure handling, portability, PMR compatibility.
- No ungrounded entity was introduced beyond spec/clarification-driven needs.
