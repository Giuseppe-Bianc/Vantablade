---
name: error-handling
description: Error handling guidelines for Vantablade
type: rule
---

# Error Handling

## Critical Paths
- `src/**`: All source code
- `include/**`: All headers

## Mandatory Baseline

- Vulkan API target: 1.4.341.1.
- C++ language target: C++23.
- Follow Khronos Vulkan specification, Vulkan Guide, Validation Layers recommendations, ISO C++23 references, cppreference, and C++ Core Guidelines.

## Error Handling Principles

- Always check Vulkan return codes and map failures to actionable diagnostics.
- Never ignore failures on resource creation, synchronization operations, command submission, or presentation.
- Propagate errors with enough context to identify failing object, operation, and stage.
- Keep cleanup idempotent and safe in partial-construction scenarios.
- Prefer deterministic failure paths over hidden retries.

## Vulkan-Specific Requirements

- Validate and handle errors for:
	- Instance and device creation.
	- Swapchain creation/recreation.
	- Pipeline creation and cache usage.
	- Buffer/image allocation, binding, mapping, and transfer.
	- Command buffer allocation, begin/end, submit, and reset.
	- Synchronization primitives creation, wait, reset, and signal paths.
- Treat validation layer errors as defects that must be fixed.
- Investigate recurring validation warnings and either fix or document technical justification.

## C++23-Specific Requirements

- Use RAII for all owned resources and ensure predictable destruction order.
- Prefer explicit error transport (`std::expected`, status objects, or well-defined exceptions policy) over ambiguous status flags.
- Keep APIs exception-safe and consistent with declared error strategy.
- Do not use undefined behavior, unchecked narrowing, or unsafe lifetime assumptions.

## Logging and Diagnostics

- Every critical failure path must emit a clear message with operation name and relevant identifiers.
- Diagnostic output should be structured enough for automated triage and reproducible debugging.
- Include synchronization context in logs when failures involve queue submit, fence wait, semaphore usage, or barrier transitions.

## Acceptance Checklist

- No unhandled Vulkan error codes in touched paths.
- No newly introduced validation layer errors in touched paths.
- No silent fallbacks that can hide functional corruption.
- Resource cleanup verified on both success and failure paths.
- Documentation updated when introducing new failure modes or recovery logic.
