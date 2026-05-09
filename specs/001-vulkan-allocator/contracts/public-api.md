# Public API Contract - VulkanAllocator Framework

Project type contract: C++ library interface contract (not REST/CLI).

## Baseline
- Vulkan API target: `1.4.341.1`
- Language baseline: C++23
- ABI notes: Vulkan callback functions follow `VKAPI_ATTR` and `VKAPI_CALL`.

## API Surface (Contracted)

### Type: `vnd::VulkanAllocator`

Required behavior:
- Not copyable and not movable.
- Deterministic initialization and shutdown.
- Thread-safe concurrent use without external synchronization.

Contracted operations:

```cpp
class VulkanAllocator final {
public:
  VulkanAllocator(/* policy objects */) noexcept;
  ~VulkanAllocator() noexcept;

  VulkanAllocator(const VulkanAllocator&) = delete;
  VulkanAllocator& operator=(const VulkanAllocator&) = delete;
  VulkanAllocator(VulkanAllocator&&) = delete;
  VulkanAllocator& operator=(VulkanAllocator&&) = delete;

  [[nodiscard]] VkAllocationCallbacks getCallbacks() noexcept;

  [[nodiscard]] AllocationStatistics snapshot() const noexcept;

  void set_label(std::string_view label) noexcept;
  void configure_debug(DebugPolicy policy) noexcept;
  void configure_failure(FailurePolicy policy) noexcept;
};
```

Notes:
- Constructor parameter shapes may evolve, but semantics above are fixed by contract.
- `getCallbacks()` must return a fully-populated callback table with `pUserData` pointing to the allocator instance.

## Vulkan Callback Contract

### `pfnAllocation`
Input contract:
- Accepts `(pUserData, size, alignment, scope)`.

Output contract:
- Returns pointer aligned to `alignment` when successful.
- Returns `nullptr` on recoverable allocation failure.

Invariants:
- Preserves `scope` metadata.
- Must not corrupt allocator state on failure.

### `pfnReallocation`
Input contract:
- Accepts `(pUserData, pOriginal, size, alignment, scope)`.

Output contract:
- Returns aligned pointer or `nullptr` on recoverable failure.
- Preserves overlapping content as required by callback semantics.

Invariants:
- Supports `pOriginal == nullptr` as allocate-equivalent behavior.
- Supports `size == 0` semantics without UB.

### `pfnFree`
Input contract:
- Accepts `(pUserData, pMemory)`.

Output contract:
- No return value.

Invariants:
- `pMemory == nullptr` is a no-op.
- Invalid/double free handling follows failure policy; unrecoverable integrity faults trigger diagnostics + termination.

### `pfnInternalAllocation` / `pfnInternalFree`
Input contract:
- Accepts Vulkan driver internal allocation notifications.

Output contract:
- Must preserve diagnostic traceability of internal allocation events.

Invariants:
- Must not interfere with external allocation correctness.

## Performance SLO Contract
- Fast path allocations (`<=64 KB`) target p99 latency `< 1 us` under 8-thread workload.
- Lightweight telemetry overhead target `< 50 ns` per allocation.
- Snapshot query target `< 10 us` under contention.

## Thread-Safety Contract
- Public methods and callback paths are safe under concurrent calls.
- Hot paths avoid global locks.
- Bounded synchronization allowed only for slow-path operations (large allocs, cross-region realloc, VM management).

## Failure Semantics Contract
- Recoverable failures: return `nullptr`.
- Unrecoverable integrity failures (misalignment, corruption, double-free): invoke diagnostic handler then terminate according to policy.
- No silent fallback on integrity violations.

## Telemetry Contract
`snapshot()` returns immutable point-in-time statistics containing at least:
- total allocations
- total allocated/deallocated bytes
- active allocations and bytes
- peak bytes
- fragmentation estimate
- per-scope distribution
- recursion-depth diagnostics

## Labeling Contract
- Labels are optional, string-based flat identifiers.
- Max label length: 64 bytes.
- Labels influence diagnostics/telemetry only, not allocation correctness.

## Portability Contract
- Behavior must remain stable across Windows/Linux/macOS supported targets.
- Platform-specific internals must not leak through public API.
- Vendor-specific optimizations must be optional, documented, and benchmark-justified.

## Validation Contract
Implementation acceptance requires:
- clean Vulkan validation-layer runs for target scenarios
- sanitizer coverage (TSAN/ASAN/UBSAN where toolchain supports)
- CTest/Catch2 passing unit and stress suites for callback semantics, alignment, and thread safety
