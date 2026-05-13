# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

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
