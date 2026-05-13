---
name: security
description: Security guidelines for Vantablade
type: rule
---

# Security

## Critical Paths
- `src/**`: All source code
- `include/**`: All headers
- `cmake/**`: Build configuration

## Mandatory Baseline

- Vulkan API target: 1.4.341.1.
- C++ language target: C++23.
- Security and correctness decisions must follow official Vulkan/Khronos documentation and recognized C++23 references.

## Security Objectives

- Preserve memory safety and synchronization correctness.
- Prevent undefined behavior and invalid API usage.
- Ensure portability and predictable behavior across supported GPU vendors, drivers, and platforms.

## Vulkan Security and Robustness Rules

- Enable and monitor validation layers in development and test environments.
- Treat validation errors as security-relevant correctness defects.
- Verify all external inputs used by Vulkan resource creation (dimensions, formats, counts, offsets, sizes).
- Enforce explicit bounds and overflow checks for allocation sizes, buffer offsets, image extents, and descriptor indexing.
- Validate synchronization assumptions to avoid data races, use-after-free, and stale resource access on GPU.
- Keep command buffer recording and submission ownership unambiguous.
- Avoid reliance on undocumented driver behavior or vendor-specific undefined semantics.

## C++23 Security and Safety Rules

- Prefer RAII and strict ownership models to prevent leaks and lifetime corruption.
- Avoid raw owning pointers; use explicit ownership abstractions.
- Favor strong types and constrained interfaces over implicit conversions.
- Minimize global mutable state and hidden side effects.
- Keep code warning-clean under strict compiler diagnostics and investigate all new warnings.

## Build and Dependency Hygiene

- Keep CMake options explicit for security-related toggles and debug validation.
- Avoid introducing dependencies without clear maintenance and security rationale.
- Pin or document critical dependency versions when behavior affects correctness or safety.

## Verification Requirements

- Run validation and debug checks for all security-sensitive code changes.
- Review touched code for memory lifetime, race conditions, unchecked arithmetic, and invalid state transitions.
- Document critical security decisions and associated technical references in project docs when introducing non-trivial changes.
