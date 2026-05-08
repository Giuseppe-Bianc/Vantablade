# Development Setup

## Prerequisites
- C++ compiler (MSVC, GCC, or Clang)
- CMake 3.20+
- Git

## Building the Project

### Windows (Visual Studio)
```bash
# Create build directory
mkdir build
cd build

# Configure with CMake Presets
cmake --preset=default

# Build
cmake --build . --config Release
```

### CMake Presets
Available presets in `CMakePresets.json`:
- `default` -- Standard debug build
- `release` -- Optimized release build
- `clang` -- Build with Clang compiler

## Dependencies
Dependencies are automatically managed by CPM. Key packages:
- **fmt** -- String formatting
- **spdlog** -- Logging library
- **GLM** -- Math library
- **GLFW** -- Window/input handling
- **Catch2** -- Testing framework

See `Dependencies.cmake` for full list and versions.

## Running Tests
```bash
# After building
ctest
```

## Project Configuration
See `ProjectOptions.cmake` for build options:
- Enable/disable sanitizers
- Compiler warnings
- Static analysis options
- Linking options

## Development Environment
Recommended setup:
- IDE: Visual Studio Code or Visual Studio
- CMake Tools extension (for VS Code)
- Clang-Format for code formatting
