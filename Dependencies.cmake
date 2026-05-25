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
      target_include_directories(imgui PUBLIC
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends"
      )
      target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)
      set_target_properties(imgui PROPERTIES FOLDER "ThirdParty")
      add_library(imgui::imgui ALIAS imgui)
    endif()
  endif()
endfunction()
