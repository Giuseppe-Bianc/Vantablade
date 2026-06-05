include(cmake/CPM.cmake)

function(AddSpdlogPackage WcharSupport WcharFilenames)
  CPMAddPackage(
          NAME spdlog
          VERSION 1.17.0
          GITHUB_REPOSITORY "gabime/spdlog"
          SYSTEM
          YES
          OPTIONS
          "SPDLOG_FMT_EXTERNAL ON"
          "SPDLOG_ENABLE_PCH ON"
          "SPDLOG_BUILD_PIC ON"
          "SPDLOG_WCHAR_SUPPORT ${WcharSupport}"
          "SPDLOG_WCHAR_FILENAMES ${WcharFilenames}"
          "SPDLOG_SANITIZE_ADDRESS OFF"
  )
endfunction()

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(Vantablade_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if(NOT TARGET fmtlib::fmtlib)
    cpmaddpackage(
      NAME
      fmt
      GITHUB_REPOSITORY
      "fmtlib/fmt"
      GIT_TAG
      "12.1.0"
      SYSTEM
      YES)
    if(TARGET fmt)
      target_compile_options(fmt PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w>
      )
      if(MSVC)
        get_target_property(_fmt_opts fmt COMPILE_OPTIONS)
        if(_fmt_opts)
          list(FILTER _fmt_opts EXCLUDE REGEX "^/W[0-4]$")
          set_target_properties(fmt PROPERTIES COMPILE_OPTIONS "${_fmt_opts}")
        endif()
        target_compile_options(fmt PRIVATE /W0)
      endif()
    endif()
  endif()

  if(NOT TARGET spdlog::spdlog)
    cpmaddpackage(
      NAME
      spdlog
      VERSION
      1.17.0
      GITHUB_REPOSITORY
      "gabime/spdlog"
      SYSTEM
      YES
      OPTIONS
      "SPDLOG_FMT_EXTERNAL ON")
  endif()

  if(NOT TARGET Catch2::Catch2WithMain)
    cpmaddpackage(
      NAME
      Catch2
      VERSION
      3.14.0
      GITHUB_REPOSITORY
      "catchorg/Catch2"
      SYSTEM
      YES)
  endif()

  if(NOT TARGET CLI11::CLI11)
    cpmaddpackage(
      NAME
      CLI11
      VERSION
      2.6.1
      GITHUB_REPOSITORY
      "CLIUtils/CLI11"
      SYSTEM
      YES)
  endif()

  if(NOT TARGET glfw)
    CPMAddPackage(
            NAME glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
            SYSTEM
            YES
            OPTIONS
            "GLFW_BUILD_EXAMPLES OFF"
            "GLFW_BUILD_TESTS OFF"
            "GLFW_BUILD_DOCS OFF"
    )
  endif()
  if (NOT TARGET glm::glm)
    CPMAddPackage(
            NAME glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG master # Use "master" for the latest version
            SYSTEM
            YES
            OPTIONS # Add options if needed
            "GLM_TEST_ENABLE OFF" # Disable tests if needed
            "GLM_ENABLE_CXX_20 ON"
            "GLM_ENABLE_SIMD_AVX2 ON"
    )
  endif ()
  if(NOT TARGET GPUOpen::VulkanMemoryAllocator)
    CPMAddPackage(
      NAME VulkanMemoryAllocator
      GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
      GIT_TAG v3.3.0
      SYSTEM YES
    )
  endif()

  if(VANTABLADE_PROFILING)
  if(NOT TARGET Tracy::TracyClient)
    CPMAddPackage(
      NAME tracy
      GITHUB_REPOSITORY wolfpld/tracy
      GIT_TAG v0.13.1
      SYSTEM
      YES
      OPTIONS
        "TRACY_ENABLE ON"
        "TRACY_ON_DEMAND ON"
    )
  endif()

  # Use ExternalProject_Add so Tracy's tools build in complete isolation.
  # Their vendor.cmake (imgui, freetype, etc.) runs in a separate CMake
  # process and cannot conflict with your project's targets or packages.
  if(DEFINED tracy_SOURCE_DIR)
    include(ExternalProject)

    # GUI profiler: output at <build>/tracy-profiler/<Config>/tracy-profiler.exe
    ExternalProject_Add(
      TracyProfiler
      SOURCE_DIR               "${tracy_SOURCE_DIR}/profiler"
      BINARY_DIR               "${CMAKE_BINARY_DIR}/tracy-profiler"
      CMAKE_GENERATOR          "${CMAKE_GENERATOR}"
      CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
      CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"
      BUILD_COMMAND
        ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG> --parallel
      INSTALL_COMMAND ""
      EXCLUDE_FROM_ALL TRUE
    )
    

    # CLI capture tool: output at <build>/tracy-capture/<Config>/tracy-capture.exe
    ExternalProject_Add(
      TracyCapture
      SOURCE_DIR               "${tracy_SOURCE_DIR}/capture"
      BINARY_DIR               "${CMAKE_BINARY_DIR}/tracy-capture"
      CMAKE_GENERATOR          "${CMAKE_GENERATOR}"
      CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
      CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"
      BUILD_COMMAND
        ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG> --parallel
      INSTALL_COMMAND ""
      EXCLUDE_FROM_ALL TRUE
    )

    # Uncomment as needed:
    # ExternalProject_Add(TracyCsvExport SOURCE_DIR "${tracy_SOURCE_DIR}/csvexport" ...)
    # ExternalProject_Add(TracyImport    SOURCE_DIR "${tracy_SOURCE_DIR}/import"    ...)
  endif()
endif()

  if(NOT TARGET imgui)
    CPMAddPackage(
      NAME imgui
      URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9.zip"
      DOWNLOAD_ONLY YES
    )
    if(imgui_ADDED)
      find_package(Vulkan REQUIRED)

      add_library(imgui STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
      )
      target_include_directories(imgui SYSTEM PUBLIC
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends"
      )
      target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)
      set_target_properties(imgui PROPERTIES FOLDER "ThirdParty")
      add_library(imgui::imgui ALIAS imgui)
    endif()
  endif()

  if (NOT TARGET tinyobjloader)
    CPMAddPackage(
            NAME tinyobjloader
            GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
            GIT_TAG release # Use "master" for the latest version
            SYSTEM
            YES
    )
    target_compile_definitions(tinyobjloader INTERFACE
      TINYOBJLOADER_DISABLE_FAST_FLOAT
    )
  endif ()
endfunction()
