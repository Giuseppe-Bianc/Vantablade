cmake_minimum_required(VERSION 3.15...4.2)

project(CmakeConfigPackageTests
        VERSION ${PROJECT_VERSION}
        LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----

if (PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)
    enable_testing()

  find_package(Vantablade CONFIG REQUIRED) # for intro, project_options, ...

  if(NOT TARGET Vantablade_options)
    message(FATAL_ERROR "Required config package not found!")
    return() # be strictly paranoid for Template Janitor github action! CK
  endif()
endif()

# ---- Dependencies ----

include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# Provide a simple smoke test to make sure that the CLI works and can display a --help message
add_test(NAME cli.has_help COMMAND vantablade --help)

# Provide a test to verify that the version being reported from the application
# matches the version given to CMake. This will be important once you package
# your program. Real world shows that this is the kind of simple mistake that is easy
# to make, but also easy to test for.
add_test(NAME cli.version_matches COMMAND vantablade --version)
set_tests_properties(cli.version_matches PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}")

# ---- Unit tests ----

add_executable(tests tests.cpp)
target_link_libraries(
  tests
  PRIVATE Vantablade::Vantablade_warnings
          Vantablade::Vantablade_options
          Vantablade::vantablade_lib
          Catch2::Catch2WithMain)

Vantablade_configure_linker(tests)

set_target_properties(tests PROPERTIES CXX_SCAN_FOR_MODULES OFF)
# Enable /bigobj flag for MSVC to handle large test files
target_compile_options(tests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
            TARGET tests
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
            COMMAND_EXPAND_LISTS)
endif ()

catch_discover_tests(
        tests
        TEST_PREFIX
        "unittests."
        REPORTER
        XML
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "unittests."
        OUTPUT_SUFFIX
        .xml)

# ---- Constexpr tests ----

add_executable(constexpr_tests constexpr_tests.cpp)
target_link_libraries(
  constexpr_tests
  PRIVATE Vantablade::Vantablade_warnings
          Vantablade::Vantablade_options
          Vantablade::vantablade_lib
          Catch2::Catch2WithMain)

Vantablade_configure_linker(constexpr_tests)
set_target_properties(constexpr_tests PROPERTIES CXX_SCAN_FOR_MODULES OFF)

catch_discover_tests(
        constexpr_tests
        TEST_PREFIX
        "constexpr."
        REPORTER
        XML
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "constexpr."
        OUTPUT_SUFFIX
        .xml)

add_executable(relaxed_constexpr_tests constexpr_tests.cpp)
target_link_libraries(
  relaxed_constexpr_tests
  PRIVATE Vantablade::Vantablade_warnings
          Vantablade::Vantablade_options
          Vantablade::vantablade_lib
          Catch2::Catch2WithMain)
target_compile_definitions(relaxed_constexpr_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

Vantablade_configure_linker(relaxed_constexpr_tests)
set_target_properties(relaxed_constexpr_tests PROPERTIES CXX_SCAN_FOR_MODULES OFF)

catch_discover_tests(
        relaxed_constexpr_tests
        TEST_PREFIX
        "relaxed_constexpr."
        REPORTER
        XML
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "relaxed_constexpr."
        OUTPUT_SUFFIX
        .xml)
