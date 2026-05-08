# Vantablade - Project Context

Vantablade is a high-performance ray tracer implemented in **C++23** using the **Vulkan API**. It features GPU-accelerated computation and efficient light transport simulation. The project is designed with a modern C++ architecture and supports cross-platform builds, including **WebAssembly** via Emscripten.

## Core Technologies

- **Language:** C++23
- **Graphics API:** Vulkan SDK 1.4.* (minimum 1.4.0)
- **Windowing/Input:** GLFW
- **Mathematics:** GLM (OpenGL Mathematics)
- **Logging:** spdlog (with async support)
- **Formatting:** {fmt} library
- **CLI Parsing:** CLI11
- **Testing:** Catch2
- **Build System:** CMake (3.29+)

## Project Structure

- `include/Vantablade/`: Public headers for the Vulkan engine (Application, Device, Pipeline, SwapChain, etc.).
- `include/VantabladeCore/`: Utilities and core functionality (Logging, File reading, Timing).
- `src/vantablade/`: Entry point application (`main.cpp`).
- `src/vantablade_lib/`: Implementation of the Vulkan engine logic.
- `src/vantablade_Core_lib/`: Implementation of utility libraries.
- `shaders/`: GLSL source and compiled SPIR-V shaders.
- `test/`: Unit and constexpr tests.

## Building and Running

### Prerequisites

- CMake 3.29 or higher.
- A C++23 compatible compiler (Clang 16+, GCC 13+, MSVC 19.36+).
- Vulkan SDK.

### Build Commands

```powershell
# Configure the project
cmake -S . -B build

# Build the project
cmake --build build --config Release

# Run the application (from the root directory to ensure shader paths are correct)
./build/src/vantablade/Release/vantablade.exe
```


### Running Tests

```powershell
cd build
ctest -C Debug
```

## Development Conventions

### Logging

Always initialize the logger at the start of `main()`:

```cpp
INIT_LOG_ASYNC(); // For asynchronous logging
// or
INIT_LOG();       // For synchronous logging
```

Use the following macros for logging:

- `LTRACE(...)`, `LDEBUG(...)`, `LINFO(...)`, `LWARN(...)`, `LERROR(...)`, `LCRITICAL(...)`

### Vulkan Error Handling

Use the `VK_CHECK` macro to wrap Vulkan function calls for consistent error reporting and assertions:
```cpp
VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device), "Failed to create logical device!");
```

### Coding Style

**Naming Conventions:**

- Classes/Structs: `PascalCase` (e.g., `VulkanDevice`, `SwapChain`)
- Functions/Methods: `camelCase` (e.g., `createDevice()`, `renderFrame()`)
- Variables: `camelCase` (e.g., `deviceCount`, `physicalDevice`)
- Constants/Enums: `UPPER_SNAKE_CASE`
- Namespaces: `lowercase` (e.g., `vantablade`, `vnd`)

**Formatting:**

- The project uses `clang-format` for automatic code formatting
- Configuration: `.clang-format` in repository root
- Run before committing: `clang-format -i file`

**C++ Best Practices:**

- Follow modern C++23 idioms (e.g., `std::views`, `std::format`).
- Prefer `const` wherever possible (const-correctness)
- Use RAII for all resource management (no raw `new`/`delete`)
- Smart pointers: `std::unique_ptr` by default, `std::shared_ptr` only when shared ownership is needed
- Use `#pragma once` instead of header guards

**Performance Profiling:**

- Use `AutoTimer` for performance profiling of scopes:

  ```cpp
  const vnd::AutoTimer timer("Heavy Operation");
  ```

- Use `DISABLE_WARNINGS_PUSH` and `DISABLE_WARNINGS_POP` for external headers or specific legacy code sections.
- The project uses `clang-tidy` and `cppcheck` for static analysis, integrated into the CMake build.

### Shaders

Shaders should be placed in the `shaders/` directory. The application expects compiled SPIR-V shaders with `.opt.spv` extension (e.g., `shaders/simple_shader.vert.opt.spv`).

## Key Files

- `src/vantablade/main.cpp`: Entry point, CLI handling, and Application startup.
- `src/vantablade_lib/Application.cpp`: Main application loop and Vulkan resource orchestration.
- `ProjectOptions.cmake`: Global build options, sanitizers, and static analysis configuration.
- `Dependencies.cmake`: External dependency management via CPM.
