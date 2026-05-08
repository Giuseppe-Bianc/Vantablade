# Vantablade Architecture

## System Design
The Vantablade project follows a modular architecture with clear separation of concerns.

### Core Components
1. **Graphics Layer** -- Vulkan-based rendering
2. **Core Engine** -- Application framework and utilities
3. **Testing Framework** -- Comprehensive test suite
4. **Build System** -- CMake-based configuration

### Dependencies
Key dependencies managed through CPM:
- fmt (formatting)
- spdlog (logging)
- GLM (mathematics)
- GLFW (windowing)
- Catch2 (testing)
- Google Test (testing)

### Module Organization
- **src/** -- Implementation files
- **include/** -- Public API headers
- **cmake/** -- Build helpers and configuration
  - `CompilerWarnings.cmake` -- Warning configuration
  - `Sanitizers.cmake` -- Sanitizer options
  - `Tests.cmake` -- Test configuration
  - `Doxygen.cmake` -- Documentation generation

## Build Targets
Multiple build targets available:
- Main executable
- Test executables
- Library targets
- Fuzz test targets

See `CMakeLists.txt` for detailed configuration.
