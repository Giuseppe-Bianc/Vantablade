# Project Overview

## Vantablade
A comprehensive C++ project with Vulkan graphics, CMake build system, and extensive testing infrastructure.

## Key Technologies
- **Language:** C++
- **Graphics API:** Vulkan
- **Build System:** CMake
- **Package Manager:** CPM (C++ Package Manager)
- **Testing:** Google Test, Catch2

## Project Structure
```
Vantablade/
├── src/              # Source code
├── include/          # Header files
├── test/             # Test suite
├── cmake/            # Build configuration
├── build/            # Build output
├── shaders/          # Shader files
└── fuzz_test/        # Fuzzing tests
```

## Build Information
- **Build Directory:** `build/`
- **CMake Presets:** Defined in `CMakePresets.json`
- **Configuration Files:** `Dependencies.cmake`, `ProjectOptions.cmake`

## Documentation
- See [[Architecture]] for system design
- See [[Development Setup]] for build instructions
