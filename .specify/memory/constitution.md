<!--
Sync Impact Report
Version change: template/unversioned -> 1.0.0
Modified principles:
- PRINCIPLE_1_NAME -> I. Vulkan Specification and Khronos Guidance First
- PRINCIPLE_2_NAME -> II. C++23 as the Language Baseline
- PRINCIPLE_3_NAME -> III. Explicit Lifetime, Ownership, and Synchronization
- PRINCIPLE_4_NAME -> IV. Validation, Diagnostics, and Reproducibility
- PRINCIPLE_5_NAME -> V. Measured Performance and Portability
Added sections:
- Platform and Dependency Baseline
- Development Workflow and Quality Gates
Removed sections:
- none
Templates requiring updates:
- .specify/templates/plan-template.md ✅ reviewed, no edits required
- .specify/templates/spec-template.md ✅ reviewed, no edits required
- .specify/templates/tasks-template.md ✅ reviewed, no edits required
- .specify/extensions/git/commands/speckit.git.initialize.md ✅ reviewed, no edits required
- .specify/extensions/git/commands/speckit.git.feature.md ✅ reviewed, no edits required
- .specify/extensions/git/commands/speckit.git.commit.md ✅ reviewed, no edits required
- .specify/extensions/git/commands/speckit.git.remote.md ✅ reviewed, no edits required
- .specify/extensions/git/commands/speckit.git.validate.md ✅ reviewed, no edits required
Deferred items:
- TODO(RATIFICATION_DATE): original adoption date is not recorded in repository history.
-->

# Vantablade Constitution

## Core Principles

### I. Vulkan Specification and Khronos Guidance First
All Vulkan-facing design and implementation decisions MUST follow the Vulkan 1.4.341.1
specification, the relevant Khronos reference material, and the validation-layer
feedback loop. Any use of extensions, features, synchronization patterns, memory
allocation strategies, command recording flow, pipeline state, or descriptor usage MUST
be justified against official documentation. Undocumented behavior, guesswork, and
vendor-specific shortcuts are prohibited unless they are isolated, explicitly documented,
and proven safe across supported targets.

### II. C++23 as the Language Baseline
All code MUST target C++23 and use modern C++23 facilities when they improve clarity,
safety, or efficiency. Ownership, lifetimes, and interfaces MUST be expressed with
explicit types and predictable semantics. Raw pointers, manual resource management, and
ad hoc helper patterns are only acceptable when they are the correct low-level
abstraction for Vulkan interop and are still wrapped in clear ownership boundaries.
Language choices SHOULD align with ISO C++23, cppreference, and the C++ Core Guidelines.

### III. Explicit Lifetime, Ownership, and Synchronization
Every GPU or CPU resource MUST have a clear owner, a defined lifetime, and a documented
destruction path. Synchronization MUST be explicit and reviewable at the point where
hazards are introduced, including image layout transitions, queue submission ordering,
fences, semaphores, barriers, and command-buffer reuse. No change may rely on implicit
synchronization or undefined ordering. Resource management code MUST make failure paths,
rollback, and partial initialization safe.

### IV. Validation, Diagnostics, and Reproducibility
Debug builds MUST enable the standard Vulkan validation layers, and issues reported by
validation tools MUST be treated as defects until explained and resolved. Logging,
assertions, and diagnostics MUST be specific enough to reproduce GPU and driver issues
without guesswork. Behavior that depends on hardware, driver, or platform differences MUST
be captured in notes, tests, or guarded code paths. A change is not complete until its
correctness can be reproduced or validated on the intended execution path.

### V. Measured Performance and Portability
Performance work MUST be evidence-driven. Any optimization to memory use, command
submission, pipeline creation, shader setup, or frame execution MUST be benchmarked or
otherwise justified against a measurable baseline. The default implementation MUST favor
portability and maintainability over fragile micro-optimizations. If a change improves one
GPU or driver but risks regressions elsewhere, the tradeoff MUST be documented and
constrained.

## Platform and Dependency Baseline

Vantablade MUST remain aligned with Vulkan 1.4.341.1 and C++23 as the supported technical
baseline. Build, debug, and release configurations MUST stay compatible with the
repository's documented toolchain and with the supported Vulkan SDK, validation layers,
and shader/tooling workflow. Features that require a newer Vulkan or C++ capability MUST
be isolated behind a documented compatibility decision.

Code that touches rendering, memory, command buffers, pipelines, or synchronization MUST
be reviewed with the Vulkan specification and the current validation-layer behavior in
mind. Cross-platform differences in GPU architecture, driver behavior, and operating-system
support MUST be treated as first-class concerns, not edge cases. If a feature is not
portable, the portability limit MUST be documented explicitly.

## Development Workflow and Quality Gates

Every feature MUST begin with a clear technical interpretation of the request and a plan
that identifies its Vulkan and C++23 implications before implementation begins. Changes
that affect rendering correctness, synchronization, memory ownership, or command recording
MUST include focused validation steps and, where appropriate, regression coverage.
Performance-sensitive changes MUST include a measurement plan or a reason for why
measurement is not yet meaningful.

Reviewers and implementers MUST verify that the code remains readable, predictable, and
maintainable after the change. Any deviation from the established architecture, coding
style, or validation approach MUST be justified in the change record. Documentation MUST
be updated when a change alters a public contract, a build requirement, a supported
platform assumption, or a debugging workflow.

## Governance

This constitution supersedes conflicting guidance in the repository when the guidance
concerns Vulkan usage, C++23 usage, resource lifetime, synchronization, validation, or
performance claims. Amendments require a written rationale, an explicit version bump, and
a review of dependent templates and workflow documents. Semantic versioning applies as
follows: MAJOR for incompatible principle or governance changes, MINOR for new or
materially expanded principles, and PATCH for clarifications or wording refinements.

All plan, spec, and task documents MUST check proposed work against this constitution
before approval. Compliance review MUST confirm that Vulkan behavior is documented or
validated, C++23 usage is appropriate, ownership and synchronization are explicit, and any
performance claim is backed by evidence. The current constitution version is 1.0.0.
Ratification date: TODO(RATIFICATION_DATE): original adoption date is not recorded in
repository history. Last amended: 2026-05-09.
