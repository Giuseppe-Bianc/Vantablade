# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:

- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:

- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:

- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:

- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:

```stext
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

## Mandatory Technical Baseline

- Language standard: C++23 (required for all new and modified C++ code).
- Graphics API baseline: Vulkan 1.4.341.1.
- All implementation choices must be aligned with:
    - Official Vulkan specification and Khronos guidance.
    - Vulkan Validation Layers and Best Practices layer output.
    - ISO C++23 language documentation, cppreference, and C++ Core Guidelines.
- Do not rely on undefined behavior, undocumented driver behavior, or vendor-specific assumptions without a documented fallback path.
- Prefer portability first, then optimization. Any optimization must be justified and validated.

## Build and Development

### Build Commands

- Generate build files: `cmake -B build`
- Build project: `cmake --build build`
- Enforce C++23 during configuration when needed: `cmake -B build -DCMAKE_CXX_STANDARD=23`

### Configuration and Validation Expectations

- Enable Vulkan validation during development and CI debug configurations.
- Treat validation errors as blockers and warnings as action items unless explicitly documented and accepted.
- Use deterministic error handling paths for Vulkan object creation and destruction.
- Verify behavior on multiple GPU vendors and driver versions when touching:
    - Memory allocation strategy.
    - Synchronization and queue ownership transfer.
    - Render pass, pipeline, descriptor, and command recording logic.

### Testing

- Enable tests: Set `BUILD_TESTING=ON` during CMake configuration
- Run all tests: `cd build && ctest`
- Run specific tests: Use `ctest -R <test_name>` from build directory
- Fuzzing: Tests in `fuzz_test/` using LibFuzzer
- Use Vulkan validation and debug tooling while running tests that exercise rendering and resource lifetime code.

### Vulkan Engineering Rules

- Memory management:
    - Follow Vulkan memory model and allocator best practices.
    - Validate alignment, memory type selection, allocation lifetime, and mapping rules.
    - Keep host/device visibility and coherency assumptions explicit and documented.
- Synchronization:
    - Prefer explicit and minimal synchronization scopes.
    - Document synchronization intent for barriers, semaphores, and fences.
    - Re-check access masks, stage masks, and image layout transitions after each pipeline-related change.
- Command recording:
    - Ensure command buffer state transitions and reset policies are valid and predictable.
    - Keep recording logic deterministic and free from hidden global state.
- Pipeline and descriptors:
    - Keep pipeline state definitions explicit and reproducible.
    - Validate descriptor set layouts, pool sizing, update frequency, and lifetime ownership.
- Performance:
    - Prefer measured optimization using profiler and validation feedback.
    - Avoid speculative micro-optimizations that reduce clarity or portability.

### C++23 Engineering Rules

- Prefer modern C++23 constructs that improve clarity and safety.
- Enforce RAII ownership semantics for Vulkan resources and system handles.
- Use `std::expected`, `std::optional`, and strong typing where they improve error clarity and API contracts.
- Avoid raw owning pointers. Use explicit ownership wrappers and smart pointers where appropriate.
- Keep APIs const-correct, exception-safe (or explicitly no-throw), and easy to reason about.
- Apply C++ Core Guidelines for lifetime safety, bounds safety, and type safety.
- Keep compile warnings clean and treat newly introduced warnings as regressions.

## Architecture

### High-Level Structure

Vantablade is a GPU-accelerated ray tracer using Vulkan.

Target baseline: Vulkan 1.4.341.1 and C++23.

- `vantablade`: Main executable entry point (`src/vantablade/main.cpp`).
- `vantablade_lib`: Core rendering logic.
    - Vulkan management: `Device.cpp`, `SwapChain.cpp`, `Pipeline.cpp`.
    - Memory: `VulkanAllocator.cpp` (via VMA).
    - Scene/Window: `Model.cpp`, `Window.cpp`.
- `vantablade_Core_lib`: Low-level utility functions.

### Flow

`main.cpp` $\rightarrow$ `Application::run()` $\rightarrow$ Vulkan Device/Pipeline Setup $\rightarrow$ Rendering Loop.

## Documentation

- Key guides: `README_dependencies.md`, `README_building.md`, `README_docker.md`.

## Critical Decision Documentation

- For high-risk or non-trivial architectural changes, document:
    - Relevant Vulkan and C++ references used for the decision.
    - Trade-offs considered.
    - Validation strategy and observed results.
    - Portability and compatibility impact across GPUs, drivers, and platforms.

<!-- SPECKIT START -->
For additional context about technologies to be used, project structure,
shell commands, and other important information, read the current plan
<!-- SPECKIT END -->
