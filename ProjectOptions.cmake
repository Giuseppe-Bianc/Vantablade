include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


include(CheckCXXSourceCompiles)


macro(Vantablade_supports_sanitizers)
  # Emscripten doesn't support sanitizers
  if(EMSCRIPTEN)
    set(SUPPORTS_UBSAN OFF)
    set(SUPPORTS_ASAN OFF)
  elseif((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)

    message(STATUS "Sanity checking UndefinedBehaviorSanitizer, it should be supported on this platform")
    set(TEST_PROGRAM "int main() { return 0; }")

    # Check if UndefinedBehaviorSanitizer works at link time
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=undefined")
    check_cxx_source_compiles("${TEST_PROGRAM}" HAS_UBSAN_LINK_SUPPORT)

    if(HAS_UBSAN_LINK_SUPPORT)
      message(STATUS "UndefinedBehaviorSanitizer is supported at both compile and link time.")
      set(SUPPORTS_UBSAN ON)
    else()
      message(WARNING "UndefinedBehaviorSanitizer is NOT supported at link time.")
      set(SUPPORTS_UBSAN OFF)
    endif()
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    if (NOT WIN32)
      message(STATUS "Sanity checking AddressSanitizer, it should be supported on this platform")
      set(TEST_PROGRAM "int main() { return 0; }")

      # Check if AddressSanitizer works at link time
      set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
      set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address")
      check_cxx_source_compiles("${TEST_PROGRAM}" HAS_ASAN_LINK_SUPPORT)

      if(HAS_ASAN_LINK_SUPPORT)
        message(STATUS "AddressSanitizer is supported at both compile and link time.")
        set(SUPPORTS_ASAN ON)
      else()
        message(WARNING "AddressSanitizer is NOT supported at link time.")
        set(SUPPORTS_ASAN OFF)
      endif()
    else()
      set(SUPPORTS_ASAN ON)
    endif()
  endif()
endmacro()

macro(Vantablade_setup_options)
  option(Vantablade_ENABLE_HARDENING "Enable hardening" ON)
  option(Vantablade_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    Vantablade_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    Vantablade_ENABLE_HARDENING
    OFF)

  Vantablade_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR Vantablade_PACKAGING_MAINTAINER_MODE)
    option(Vantablade_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(Vantablade_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(Vantablade_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(Vantablade_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(Vantablade_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(Vantablade_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(Vantablade_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(Vantablade_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(Vantablade_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(Vantablade_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(Vantablade_ENABLE_PCH "Enable precompiled headers" OFF)
    option(Vantablade_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(Vantablade_ENABLE_IPO "Enable IPO/LTO" ON)
    option(Vantablade_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(Vantablade_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(Vantablade_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(Vantablade_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(Vantablade_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(Vantablade_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(Vantablade_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(Vantablade_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(Vantablade_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(Vantablade_ENABLE_PCH "Enable precompiled headers" OFF)
    option(Vantablade_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      Vantablade_ENABLE_IPO
      Vantablade_WARNINGS_AS_ERRORS
      Vantablade_ENABLE_SANITIZER_ADDRESS
      Vantablade_ENABLE_SANITIZER_LEAK
      Vantablade_ENABLE_SANITIZER_UNDEFINED
      Vantablade_ENABLE_SANITIZER_THREAD
      Vantablade_ENABLE_SANITIZER_MEMORY
      Vantablade_ENABLE_UNITY_BUILD
      Vantablade_ENABLE_CLANG_TIDY
      Vantablade_ENABLE_CPPCHECK
      Vantablade_ENABLE_COVERAGE
      Vantablade_ENABLE_PCH
      Vantablade_ENABLE_CACHE)
  endif()

  Vantablade_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (Vantablade_ENABLE_SANITIZER_ADDRESS OR Vantablade_ENABLE_SANITIZER_THREAD OR Vantablade_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(Vantablade_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})
  option(VANTABLADE_PROFILING "Enable Tracy profiling" OFF)

endmacro()

macro(Vantablade_global_options)
  include(cmake/Simd.cmake)
  check_all_simd_features()
  print_simd_support()
  if(Vantablade_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    Vantablade_enable_ipo()
  endif()

  Vantablade_supports_sanitizers()

  if(Vantablade_ENABLE_HARDENING AND Vantablade_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR Vantablade_ENABLE_SANITIZER_UNDEFINED
       OR Vantablade_ENABLE_SANITIZER_ADDRESS
       OR Vantablade_ENABLE_SANITIZER_THREAD
       OR Vantablade_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${Vantablade_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${Vantablade_ENABLE_SANITIZER_UNDEFINED}")
    Vantablade_enable_hardening(Vantablade_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(Vantablade_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(Vantablade_warnings INTERFACE)
  add_library(Vantablade_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  Vantablade_set_project_warnings(
    Vantablade_warnings
    ${Vantablade_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  include(cmake/Linker.cmake)
  # Must configure each target with linker options, we're avoiding setting it globally for now

  if(NOT EMSCRIPTEN)
    include(cmake/Sanitizers.cmake)
    Vantablade_enable_sanitizers(
      Vantablade_options
      ${Vantablade_ENABLE_SANITIZER_ADDRESS}
      ${Vantablade_ENABLE_SANITIZER_LEAK}
      ${Vantablade_ENABLE_SANITIZER_UNDEFINED}
      ${Vantablade_ENABLE_SANITIZER_THREAD}
      ${Vantablade_ENABLE_SANITIZER_MEMORY})
  endif()

  set_target_properties(Vantablade_options PROPERTIES UNITY_BUILD ${Vantablade_ENABLE_UNITY_BUILD})

  if(Vantablade_ENABLE_PCH)
    target_precompile_headers(
      Vantablade_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(Vantablade_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    Vantablade_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(Vantablade_ENABLE_CLANG_TIDY)
    Vantablade_enable_clang_tidy(Vantablade_options ${Vantablade_WARNINGS_AS_ERRORS})
  endif()

  if(Vantablade_ENABLE_CPPCHECK)
    Vantablade_enable_cppcheck(${Vantablade_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(Vantablade_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    Vantablade_enable_coverage(Vantablade_options)
  endif()

  if(Vantablade_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(Vantablade_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(Vantablade_ENABLE_HARDENING AND NOT Vantablade_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR Vantablade_ENABLE_SANITIZER_UNDEFINED
       OR Vantablade_ENABLE_SANITIZER_ADDRESS
       OR Vantablade_ENABLE_SANITIZER_THREAD
       OR Vantablade_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    Vantablade_enable_hardening(Vantablade_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
