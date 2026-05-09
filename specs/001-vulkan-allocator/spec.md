# Feature Specification: High-Performance Vulkan CPU Memory Allocator Framework

**Feature Branch**: `001-vulkan-allocator`  
**Created**: May 9, 2026  
**Status**: Draft  
**Input**: User description: Build a high performance CPU side memory allocator framework for Vulkan 1.4.341.1

## User Scenarios & Testing

### User Story 1 - Initialize Allocator with Custom Policies (Priority: P1)

Graphics engine developers need to initialize allocator instances with specific allocation policies suitable for their rendering workload characteristics (immediate mode vs deferred rendering, streaming vs static assets, high frequency reallocation vs stable allocations).

**Why this priority**: Core initialization is prerequisite for all other functionality. Without proper policy configuration, the allocator cannot be deployed in production environments.

**Independent Test**: Can be fully tested by instantiating an allocator with various policy configurations (object pool size, slab bucket strategy, fragmentation thresholds) and verifying that policies take effect through telemetry inspection without requiring any Vulkan instance creation.

**Acceptance Scenarios**:

1. **Given** an allocator class with default configuration, **When** initialized with custom size class policies, **Then** telemetry reports correctly reflect the configured policies
2. **Given** a developer wanting small allocation optimization, **When** configuring slab pool for allocations under 4KB, **Then** all subsequent small allocations use slab strategy
3. **Given** transient workload requirements, **When** configuring linear arena allocator with custom reset frequency, **Then** allocator supports efficient frame-scoped allocations
4. **Given** NUMA aware system, **When** specifying NUMA node affinity policies, **Then** allocator respects node constraints in virtual memory reservation

---

### User Story 2 - Register VkAllocationCallbacks with Vulkan Objects (Priority: P1)

Vulkan application developers need to register fully compliant VkAllocationCallbacks with Vulkan instances and devices so that all Vulkan runtime allocations flow through the custom allocator framework.

**Why this priority**: Integration with Vulkan is mandatory for the framework to be useful. Without proper callback registration, Vulkan continues using default allocation behavior.

**Independent Test**: Can be fully tested by creating a Vulkan instance with registered callbacks, triggering basic allocations through simple Vulkan operations (instance layer allocations, queue queries), and verifying callbacks were invoked with correct parameters (sizes, alignments, scopes).

**Acceptance Scenarios**:

1. **Given** an initialized allocator instance, **When** creating VkAllocationCallbacks struct and registering with vkCreateInstance, **Then** subsequent Vulkan operations invoke pfnAllocation callback
2. **Given** Vulkan instance with registered callbacks, **When** creating logical device, **Then** device creation allocations flow through pfnAllocation with VK_SCOPE_DEVICE scope
3. **Given** command buffer recording, **When** allocating command buffer memory, **Then** pfnAllocation invoked with VK_SCOPE_COMMAND scope
4. **Given** object destruction, **When** destroying Vulkan objects, **Then** pfnFree callback invoked with exact allocation pointers previously returned

---

### User Story 3 - Safe Multithreaded Allocation and Deallocation (Priority: P1)

Rendering engines operating multiple CPU worker threads need to safely allocate and deallocate memory from the allocator without explicit synchronization, guaranteeing thread safety and lock-free behavior in hot paths.

**Why this priority**: Thread safety is fundamental to deployment in multithreaded Vulkan engines. Without safe concurrent access, developers must implement external synchronization defeating low latency goals.

**Independent Test**: Can be fully tested by launching multiple threads performing randomized allocation/deallocation workloads, monitoring for data races using ThreadSanitizer, verifying all allocations remain valid, and confirming no external synchronization is required.

**Acceptance Scenarios**:

1. **Given** 8 concurrent threads, **When** each thread performs 1000 random sized allocations and deallocations, **Then** all operations complete without deadlock and all returned pointers remain valid
2. **Given** producer-consumer allocation pattern (one thread allocating, another freeing), **When** running indefinitely with randomized timing, **Then** no external locks required and latency remains predictable
3. **Given** burst allocation pattern (rapid allocations followed by bulk deallocation), **When** executed from multiple threads, **Then** allocator remains stable and telemetry reports correct active allocation count
4. **Given** ThreadSanitizer enabled compilation, **When** running stress tests, **Then** zero data race warnings reported

---

### User Story 4 - Inspect Allocation Telemetry (Priority: P2)

Graphics profilers and performance analysts need access to comprehensive allocation metrics enabling real-time performance monitoring and bottleneck identification.

**Why this priority**: Profiling is essential for optimization but not required for basic functionality. Can be added in follow-up phase.

**Independent Test**: Can be fully tested by instrumenting allocator with known allocation patterns (fixed size allocations, random patterns, fragmentation scenarios) and validating telemetry accuracy through snapshot comparison.

**Acceptance Scenarios**:

1. **Given** allocator performing known allocation sequence, **When** querying telemetry snapshot, **Then** reported allocation count matches expected value exactly
2. **Given** active allocations totaling 256MB, **When** querying current memory usage, **Then** telemetry returns 256MB precisely
3. **Given** fragmentation scenario with interleaved allocations, **When** computing fragmentation metric, **Then** reports percentage aligned with theoretical fragmentation level
4. **Given** multithreaded workload, **When** querying per-scope allocation distribution, **Then** breakdown by VK_SCOPE_* matches allocation patterns

---

### User Story 5 - Configure Debug Features Independently (Priority: P2)

Debug teams need to enable/disable specific debugging features (leak detection, double-free checking, guard regions) to investigate allocator issues without requiring full recompilation.

**Why this priority**: Debugging capability accelerates issue diagnosis but not required for production deployment. Should be available through configuration channels.

**Independent Test**: Can be fully tested by initializing allocator with debug configuration flags, intentionally triggering errors (double-free, leaks), and verifying errors are detected and reported.

**Acceptance Scenarios**:

1. **Given** allocator with leak detection enabled, **When** allocating memory and intentionally not freeing, **Then** shutdown diagnostics report leaked allocation with origin information
2. **Given** allocator with double-free detection enabled, **When** freeing same pointer twice, **Then** allocator terminates with explicit diagnostic message
3. **Given** allocator with guard regions enabled, **When** corrupting guard region via memory access, **Then** next allocation or diagnostic check detects corruption
4. **Given** debug features disabled, **When** running release build, **Then** no performance overhead from debug instrumentation

---

### User Story 6 - Integrate with std::pmr Containers (Priority: P3)

C++ component libraries need to use allocator framework through std::pmr compatibility layer, enabling integration with standard containers without reimplementation.

**Why this priority**: PMR integration enables broader ecosystem support but not required for core Vulkan integration. Can be added after primary functionality stabilizes.

**Independent Test**: Can be fully tested by creating std::pmr-compatible resource adapter, allocating std::vector and other containers through the resource, and verifying allocations flow through allocator framework with preserved metadata.

**Acceptance Scenarios**:

1. **Given** pmr resource adapter wrapping allocator instance, **When** constructing std::vector with pmr resource, **Then** vector allocations flow through underlying allocator callbacks
2. **Given** nested containers using pmr resource, **When** resizing container to trigger reallocations, **Then** pfnReallocation callback invoked with correct parameters
3. **Given** multiple containers sharing pmr resource, **When** destroying containers, **Then** all allocations properly released in correct order
4. **Given** telemetry collection enabled, **When** using pmr containers, **Then** allocations remain tracked and visible through profiling interfaces

---

### Edge Cases

- What happens when allocation request exceeds available virtual address space on the platform?
- How does allocator handle Vulkan scope transitions (e.g., object created in INSTANCE scope then destroyed in DEVICE scope)?
- How does allocator behave when pfnAllocation callback is invoked recursively during allocation metadata management?
- What happens when platform page protection features (e.g., ASLR, DEP) interact with large region allocation strategies?
- How does allocator handle allocation requests with exotic alignment values (e.g., 256-byte cache line alignment on 4KB page boundary systems)?
- What happens under extreme memory pressure when virtual memory reservation succeeds but commitment fails?
- How does allocator behave when operating in environments with strict SELinux or similar security policies?

## Requirements

### Functional Requirements

**Core Allocation Callback Compliance**

- **FR-001**: Allocator MUST implement pfnAllocation callback accepting size, alignment, scope, and user data according to Vulkan specification semantics and return properly aligned memory pointer
- **FR-002**: Allocator MUST implement pfnReallocation callback supporting reallocation of existing allocations to new size while preserving content for overlapping region as specified in Vulkan specification
- **FR-003**: Allocator MUST implement pfnFree callback accepting allocation pointer and user data, releasing associated resources, and handling invalid pointers per Vulkan specification expectations
- **FR-004**: Allocator MUST implement pfnInternalAllocation callback for Vulkan driver internal allocation notifications preserving scope and size information for diagnostics
- **FR-005**: Allocator MUST implement pfnInternalFree callback matching pfnInternalAllocation semantics for driver internal deallocation tracking
- **FR-006**: All callbacks MUST remain accessible through standard VkAllocationCallbacks structure suitable for direct use with vkCreateInstance, vkCreateDevice, and related Vulkan functions
- **FR-007**: Allocator MUST preserve all Vulkan allocation scopes (VK_SCOPE_COMMAND, VK_SCOPE_OBJECT, VK_SCOPE_CACHE, VK_SCOPE_DEVICE, VK_SCOPE_INSTANCE) as provided by Vulkan callbacks
- **FR-008**: Allocator MUST enforce strict Vulkan alignment guarantees, treating misaligned allocation results as fatal allocator errors

**Thread Safety and Concurrency**

- **FR-009**: Allocator MUST support safe concurrent allocation and deallocation from unlimited threads without requiring user-provided external synchronization
- **FR-010**: Allocator MUST provide lock-free or wait-free fast paths for allocation operations in scenarios where platform synchronization primitives allow
- **FR-011**: Allocator MUST maintain bounded, predictable latency under multithreaded concurrent workloads with measurable contention metrics
- **FR-012**: Allocator MUST preserve allocation ownership and metadata correctness under all supported concurrency conditions without data races
- **FR-013**: Allocator MUST support efficient allocation reuse patterns across threads without global contention in hot allocation paths

**Allocation Strategy Framework**

- **FR-014**: Allocator MUST support configurable small allocation strategy using cache-friendly slab or pool-based mechanisms minimizing fragmentation
- **FR-015**: Allocator MUST support configurable medium allocation strategy using scalable region or segregated fit allocation techniques
- **FR-016**: Allocator MUST support configurable large allocation strategy using direct virtual memory backed regions with efficient release semantics
- **FR-017**: Allocator MUST support transient/frame-local allocation strategy using resettable linear or arena allocation optimized for throughput
- **FR-018**: Allocator MUST provide deterministic allocation strategy selection observable through diagnostic interfaces
- **FR-019**: Allocator MUST minimize internal fragmentation through size class segregation and efficient packing algorithms

**Memory Alignment**

- **FR-020**: Allocator MUST respect all Vulkan requested alignment values returning strictly aligned allocations
- **FR-021**: Allocator MUST support configurable CPU cache line alignment (typically 64 bytes) when requested by policies
- **FR-022**: Allocator MUST support SIMD alignment constraints (16, 32, or 64 bytes) when requested through allocation configuration
- **FR-023**: Allocator MUST respect platform virtual memory granularity requirements (typically 4KB on x86/ARM systems)
- **FR-024**: Allocator MUST treat alignment violations as fatal internal errors, never silently degrading into undefined behavior

**Debugging and Diagnostics**

- **FR-025**: Allocator MUST provide configurable allocation tracking enabling identification of all active allocations with origin metadata
- **FR-026**: Allocator MUST support leak detection identifying allocations not freed before allocator shutdown
- **FR-027**: Allocator MUST support double-free detection identifying attempts to free same pointer multiple times
- **FR-028**: Allocator MUST support invalid-free detection identifying attempts to free unallocated or invalid pointers
- **FR-029**: Allocator MUST track allocation lifetime from creation to destruction enabling analysis of allocation duration patterns
- **FR-030**: Allocator MUST track allocation origin (source file, line number, function name) enabling debugging and profiling
- **FR-031**: Allocator MUST maintain allocation scope tracing preserving Vulkan scope information throughout allocation lifetime
- **FR-032**: Allocator MUST compute allocation size histograms enabling analysis of allocation size distributions
- **FR-033**: Allocator MUST provide fragmentation metrics estimating internal and external fragmentation levels
- **FR-034**: Allocator MUST support optional guard region validation detecting heap corruption attempts
- **FR-035**: Debug functionality MUST remain configurable separating development instrumentation from release performance builds

**Profiling and Telemetry**

- **FR-036**: Allocator MUST expose allocation count metrics tracking total allocations performed
- **FR-037**: Allocator MUST expose allocation size metrics tracking total bytes allocated and deallocated
- **FR-038**: Allocator MUST expose active allocation totals tracking current number and bytes of active allocations
- **FR-039**: Allocator MUST expose peak memory usage metrics tracking maximum memory usage since allocator creation
- **FR-040**: Allocator MUST expose thread contention metrics indicating synchronization overhead and lock hold times
- **FR-041**: Allocator MUST expose fragmentation estimates through internal/external fragmentation percentages
- **FR-042**: Allocator MUST expose allocation latency statistics tracking minimum, maximum, mean, and percentile latencies
- **FR-043**: Allocator MUST expose allocation distribution across Vulkan scopes enabling scope-specific analysis
- **FR-044**: Profiling collection MUST support low overhead operation avoiding mandatory global synchronization in hot paths
- **FR-045**: Allocator MUST support telemetry snapshot queries enabling consistent multi-metric reads

**Failure Semantics**

- **FR-046**: Allocator MUST propagate allocation failures deterministically returning null or appropriate error status through Vulkan callback expectations
- **FR-047**: Allocator MUST handle out-of-memory conditions safely without corrupting allocator state
- **FR-048**: Allocator MUST detect and report invalid alignment requests with explicit diagnostics
- **FR-049**: Allocator MUST detect and report corrupted metadata with diagnostic information enabling recovery
- **FR-050**: Allocator MUST detect and report unsupported allocation patterns with clear error messages
- **FR-051**: Allocator MUST detect and report synchronization violations with structured error context
- **FR-052**: Allocator MUST support configurable fail-fast behavior for debug configurations enabling immediate error detection
- **FR-053**: Fatal allocator integrity violations MUST produce explicit diagnostics and never silently continue

**Memory Tagging and Labeling**

- **FR-054**: Allocator MUST support configurable memory tagging and allocation labeling for debugging and profiling
- **FR-055**: Allocator MUST enable association of allocation labels with Vulkan objects enabling object-centric profiling
- **FR-056**: Allocator MUST enable association of allocation labels with subsystems enabling subsystem-level telemetry
- **FR-057**: Allocator MUST enable association of allocation labels with rendering stages enabling stage-level analysis
- **FR-058**: Allocator MUST enable association of allocation labels with engine-level ownership groups
- **FR-059**: Allocator MUST support queryable label interfaces enabling diagnostics without affecting allocation correctness

**Platform Portability**

- **FR-060**: Allocator MUST maintain portability across Linux, Windows, and macOS environments
- **FR-061**: Allocator MUST support MoltenVK-based Vulkan implementations on Apple platforms through compatibility layers
- **FR-062**: Allocator MUST provide platform abstraction layers isolating operating system specific virtual memory operations
- **FR-063**: Allocator MUST abstract page protection behavior differences across platforms
- **FR-064**: Allocator MUST abstract NUMA considerations for multi-socket systems
- **FR-065**: Allocator MUST abstract thread-local storage primitives across platforms
- **FR-066**: Allocator MUST abstract atomic synchronization facilities across platforms
- **FR-067**: Platform-specific behavior MUST never leak into public allocator APIs

**Virtual Memory Management**

- **FR-068**: Allocator MUST support configurable virtual memory reservation strategies suitable for graphics workloads
- **FR-069**: Allocator MUST support configurable virtual memory commitment strategies separating reservation from commitment
- **FR-070**: Large region management MUST minimize operating system allocation churn through region reuse
- **FR-071**: Allocator MUST support efficient reuse of previously reserved regions avoiding repeated OS allocation calls
- **FR-072**: Virtual memory handling MUST remain compliant with platform APIs avoiding assumptions about contiguous physical memory
- **FR-073**: Allocator MUST support sparse virtual memory allocation patterns where platform APIs enable

**Initialization and Shutdown**

- **FR-074**: Allocator MUST provide deterministic initialization behavior with configurable policies applied during construction
- **FR-075**: Allocator MUST provide deterministic shutdown behavior releasing all owned regions and metadata pools
- **FR-076**: Allocator MUST release all synchronization primitives during shutdown without deadlock
- **FR-077**: Allocator MUST release all telemetry structures during shutdown preserving final statistics
- **FR-078**: Allocator MUST release all debug tracking systems during shutdown
- **FR-079**: Shutdown validation MUST detect unreleased allocations providing structured diagnostics
- **FR-080**: Allocator MUST support structured diagnostics describing allocation origins and ownership metadata during shutdown

**std::pmr Compatibility**

- **FR-081**: Allocator MUST provide dedicated polymorphic memory resource adapters enabling std::pmr integration
- **FR-082**: PMR integration MUST preserve allocator statistics through wrapped callbacks
- **FR-083**: PMR integration MUST preserve debugging metadata through allocation labels
- **FR-084**: PMR integration MUST preserve alignment guarantees through constraint propagation
- **FR-085**: PMR integration MUST preserve allocation scopes through metadata attachment
- **FR-086**: PMR integration MUST preserve profiling visibility through telemetry hooks
- **FR-087**: PMR resources MUST remain interoperable with standard C++23 containers
- **FR-088**: PMR resources MUST respect allocator propagation rules in container operations

**API Design**

- **FR-089**: Public API MUST follow modern C++23 design principles and C++ Core Guidelines
- **FR-090**: Public API MUST use explicit ownership semantics avoiding raw pointer ambiguity
- **FR-091**: Public API MUST use RAII patterns for resource management
- **FR-092**: Public API MUST use constexpr where appropriate enabling compile-time evaluation
- **FR-093**: Public API MUST use noexcept correctness indicating non-throwing operations
- **FR-094**: Public API MUST use strong typing avoiding implicit conversions
- **FR-095**: Public API MUST use span-based memory views instead of raw pointers
- **FR-096**: Public API MUST use enum class instead of unscoped enums
- **FR-097**: Public API MUST use standard atomics from <atomic> for synchronization
- **FR-098**: Public API MUST use standard synchronization primitives where applicable
- **FR-099**: Public API MUST support allocator-aware container integration patterns
- **FR-100**: Public API MUST avoid undefined lifetime coupling and hidden global state

**Module Architecture**

- **FR-101**: Allocator framework MUST expose clear separation between public interfaces and internal strategies
- **FR-102**: Allocator framework MUST expose clear separation between allocation strategies and platform layers
- **FR-103**: Allocator framework MUST expose clear separation between synchronization mechanisms and allocation logic
- **FR-104**: Allocator framework MUST expose clear separation between debugging systems and core allocation
- **FR-105**: Allocator framework MUST expose clear separation between telemetry systems and allocation operations
- **FR-106**: Internal modules MUST remain independently testable
- **FR-107**: Internal modules MUST remain replaceable without affecting public API
- **FR-108**: Cross-module dependencies MUST remain minimal and explicit
- **FR-109**: Circular dependencies MUST not exist between major modules

**Testing and Validation**

- **FR-110**: Allocator MUST be testable under Vulkan validation layers without diagnostic errors
- **FR-111**: Allocator MUST be testable with ThreadSanitizer enabling data race detection
- **FR-112**: Allocator MUST be testable with AddressSanitizer enabling memory access validation
- **FR-113**: Allocator MUST be testable with UndefinedBehaviorSanitizer enabling undefined behavior detection
- **FR-114**: Allocator MUST support stress testing for multithreaded contention
- **FR-115**: Allocator MUST support stress testing for fragmentation resistance over long durations
- **FR-116**: Allocator MUST support stress testing for allocation churn patterns
- **FR-117**: Allocator MUST support stress testing for transient allocation bursts
- **FR-118**: Allocator MUST support stress testing with randomized allocation patterns
- **FR-119**: Allocator MUST support failure injection scenarios for robustness validation

### Key Entities

**VkAllocationCallbacks Integration**

- **Allocation Context**: Encapsulates allocator instance and configuration accessible through pUserData in VkAllocationCallbacks
- **Allocation Metadata**: Stores pointer validation, size, alignment, scope, origin, lifetime tracking, and debug information
- **Scope Tracking**: Maps VkSystemAllocationScope to internal allocation category enabling scope-specific management

**Allocation Strategy**

- **Slab Allocator**: Manages small allocations (typically <4KB) through fixed-size buckets minimizing fragmentation
- **Region Allocator**: Manages medium allocations (4KB-1MB) through segregated fit or buddy system allocation
- **Large Allocator**: Manages large allocations (>1MB) through direct virtual memory backed regions
- **Linear/Arena Allocator**: Manages transient allocations through resettable linear allocation optimized for frame scoping

**Synchronization Structures**

- **Atomic Operations**: Uses std::atomic primitives for lock-free reference counting and state tracking
- **Spin Locks**: Lightweight synchronization for fast paths where contention expected minimal
- **Rw Locks**: Reader-writer locks for read-heavy telemetry access patterns
- **Condition Variables**: Signaling mechanisms for synchronization points when necessary

**Telemetry Structures**

- **Statistics Snapshot**: Immutable view of allocator metrics at specific point in time
- **Per-Thread Counters**: Thread-local accounting enabling efficient contention-free updates
- **Global Aggregator**: Collects per-thread counters into unified metrics
- **Scope Distribution**: Breakdown of metrics across VkSystemAllocationScope categories

**Debug Structures**

- **Allocation Record**: Stores origin file/line, allocation timestamp, lifetime info, guard regions, and payload
- **Leak Tracker**: Maintains set of active allocations enabling leak detection at shutdown
- **Double-Free Detector**: Tracks freed pointers enabling detection of reuse attempts
- **Guard Region**: Pattern before/after allocation to detect out-of-bounds writes

## Success Criteria

### Performance Metrics

- **SC-001**: Single-threaded allocation latency must average under 500 nanoseconds for small allocations
- **SC-002**: Single-threaded allocation latency must average under 5 microseconds for large allocations (>1MB)
- **SC-003**: Multithreaded allocation throughput must exceed 1 million allocations/second aggregate across 8 concurrent threads
- **SC-004**: Memory fragmentation overhead must remain under 15% for typical allocation patterns
- **SC-005**: Lock contention on fast paths must remain sub-microsecond under sustained 8-thread concurrent workload

### Correctness Metrics

- **SC-006**: All Vulkan callback parameters must exactly match callback invocations for 100% of allocations
- **SC-007**: All returned allocations must satisfy requested alignment constraints for 100% of allocations
- **SC-008**: Allocation pointer validity must be maintained across reallocation operations for 100% of reallocations
- **SC-009**: Telemetry accuracy must match actual allocation state within 0.1% variance across all metrics
- **SC-010**: Zero data races under ThreadSanitizer during multithreaded stress testing

### Feature Completeness Metrics

- **SC-011**: 100% of Vulkan callback semantics must be implemented and tested
- **SC-012**: 100% of Vulkan allocation scopes must be supported and traceable
- **SC-013**: 95% of user stories must be independently implementable and testable before plan completion
- **SC-014**: 100% of debug features must be independently configurable and effective

### Reliability Metrics

- **SC-015**: Allocator must successfully handle 10 million+ allocation/deallocation cycles without corruption
- **SC-016**: Allocator must successfully handle up to 10GB total allocation volume without instability
- **SC-017**: Allocator must gracefully handle platform out-of-memory conditions without undefined behavior
- **SC-018**: Shutdown diagnostics must detect 100% of resource leaks with accurate origin information

### Cross-Platform Metrics

- **SC-019**: Allocator behavior must remain consistent across Linux, Windows, and macOS
- **SC-020**: Performance variance between platforms must remain under 20% for equivalent workloads

## Assumptions

- **Vulkan Minimum Version**: Target Vulkan 1.4.341.1 with compatibility consideration for 1.3.x versions
- **C++ Standard**: C++23 or later required for deployment; C++20 minimum for build compatibility
- **Platform Support**: Linux (glibc 2.29+), Windows (Windows 10 20H2+), macOS (10.15+)
- **Physical Memory Availability**: Allocator assumes sufficient virtual address space available (minimum 1GB reserve)
- **Processor Capabilities**: Assumes x86-64, ARM64, or equivalent with atomic operations support
- **Threading Model**: Allocator assumes POSIX-compatible threading on Linux/macOS; Windows threading on Windows
- **Memory Layout**: Assumes all memory accessed through standard pointer semantics with no custom memory translation
- **Vulkan Semantics**: Callbacks assume standard Vulkan allocation contract semantics unchanged across versions

## Out of Scope

- GPU device memory allocation (separate from CPU allocation)
- Vulkan resource allocators equivalent to VMA
- Custom graphics APIs unrelated to Vulkan
- Garbage collection mechanisms
- Scripting runtime allocators
- Distributed memory systems
- Persistent storage allocators
