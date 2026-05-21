include(GenerateExportHeader)

find_package(Vulkan REQUIRED)
add_library(vantablade_lib
        vantablade.cpp
        ../../include/Vantablade/WindowCallback.hpp
        ../../include/Vantablade/vulkanToString.hpp
        ../../include/Vantablade/vulkanCheck.hpp
        ../../include/Vantablade/VulkanLogInfoCallback.hpp
        Window.cpp
        ../../include/Vantablade/Window.hpp
        Monitor.cpp
        ../../include/Vantablade/Monitor.hpp
        VmaImplementation.cpp
        Application.cpp
        ../../include/Vantablade/Application.hpp
        ../../include/Vantablade/VkObjectTypeResolve.hpp
        FPSCounter.cpp
        ../../include/Vantablade/FPSCounter.hpp
        ../../include/Vantablade/glm_matld.hpp
        ../../include/Vantablade/glm_prety_string_cast.hpp
        Pipeline.cpp
        ../../include/Vantablade/Pipeline.hpp
        Device.cpp
        ../../include/Vantablade/Device.hpp
        SwapChain.cpp
        ../../include/Vantablade/SwapChain.hpp
        Model.cpp
        ../../include/Vantablade/Model.hpp
        GameObject.cpp
        ../../include/Vantablade/GameObject.hpp
)


add_library(Vantablade::vantablade_lib ALIAS vantablade_lib)

get_target_property(target_name vantablade_lib NAME)
include("${CMAKE_SOURCE_DIR}/cmake/Simd.cmake")
set_simd_instructions(${target_name})

target_link_libraries(vantablade_lib
        PRIVATE
        Vantablade_options
        Vantablade_warnings
        PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
        GPUOpen::VulkanMemoryAllocator
        Vantablade::vantablade_core_lib)

target_include_directories(vantablade_lib ${WARNING_GUARD}
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/configured_files/include>
)

if (Vantablade_BUILD_FUZZ_TESTS)
    target_link_libraries(vantablade_lib PRIVATE -fsanitize=fuzzer-no-link)
    target_compile_options(vantablade_lib PRIVATE -fsanitize=fuzzer-no-link)
endif ()

target_compile_features(vantablade_lib PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

Vantablade_configure_linker(vantablade_lib)

set_target_properties(
        vantablade_lib
        PROPERTIES VERSION ${PROJECT_VERSION}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES)

generate_export_header(vantablade_lib EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/Vantablade/vantablade_export.hpp)

############## Build SHADERS #######################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS
        ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
        /usr/bin
        /usr/local/bin
        ${VULKAN_SDK_PATH}/Bin
        ${VULKAN_SDK_PATH}/Bin32
        $ENV{VULKAN_SDK}/Bin/
        $ENV{VULKAN_SDK}/Bin32/
)

if (NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found - Required for GLSL compilation")
endif ()
message(STATUS "Using glslangValidator: ${GLSL_VALIDATOR}")

find_program(SPIRV_OPT spirv-opt
        HINTS
        /usr/bin
        /usr/local/bin
        ${VULKAN_SDK_PATH}/Bin
        ${VULKAN_SDK_PATH}/Bin32
        $ENV{VULKAN_SDK}/Bin/
        $ENV{VULKAN_SDK}/Bin32/
)
if (SPIRV_OPT)
    message(STATUS "SPIR-V optimizer found: ${SPIRV_OPT}")
else ()
    message(WARNING "spirv-opt not found - Skipping SPIR-V optimization")
endif ()


# get all .vert and .frag files in shaders directory
set(SHADER_ROOT_DIR "${PROJECT_SOURCE_DIR}/shaders")
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${SHADER_ROOT_DIR}/*.vert" "${SHADER_ROOT_DIR}/*.frag"
        "${SHADER_ROOT_DIR}/*.comp" "${SHADER_ROOT_DIR}/*.geom"
        "${SHADER_ROOT_DIR}/*.tesc" "${SHADER_ROOT_DIR}/*.tese"
        "${SHADER_ROOT_DIR}/*.rgen" "${SHADER_ROOT_DIR}/*.rahit"
        "${SHADER_ROOT_DIR}/*.rchit" "${SHADER_ROOT_DIR}/*.rmiss"
        "${SHADER_ROOT_DIR}/*.rcall"
)

if (NOT GLSL_SOURCE_FILES)
    message(WARNING "No GLSL shader sources found in ${SHADER_ROOT_DIR}")
else ()
    list(LENGTH GLSL_SOURCE_FILES NUM_SHADERS)
    message(STATUS "Found ${NUM_SHADERS} shader files:")
    foreach (shader ${GLSL_SOURCE_FILES})
        file(RELATIVE_PATH rel_path ${SHADER_ROOT_DIR} ${shader})
        message(STATUS "  ${rel_path}")
    endforeach ()
endif ()

set(VANTABLADE_VULKAN_TARGET "" CACHE STRING "Override Vulkan target environment")

if (NOT VANTABLADE_VULKAN_TARGET)
    if (Vulkan_VERSION VERSION_GREATER_EQUAL "1.4.0")
        set(VANTABLADE_VULKAN_TARGET "vulkan1.4")
    elseif (Vulkan_VERSION VERSION_GREATER_EQUAL "1.3.0")
        set(VANTABLADE_VULKAN_TARGET "vulkan1.3")
    elseif (Vulkan_VERSION VERSION_GREATER_EQUAL "1.2.0")
        set(VANTABLADE_VULKAN_TARGET "vulkan1.2")
    elseif (Vulkan_VERSION VERSION_GREATER_EQUAL "1.1.0")
        set(VANTABLADE_VULKAN_TARGET "vulkan1.1")
    else ()
        set(VANTABLADE_VULKAN_TARGET "vulkan1.0")
    endif ()
endif ()

message(STATUS "Using Vulkan target env: ${VANTABLADE_VULKAN_TARGET}")

foreach (GLSL ${GLSL_SOURCE_FILES})
    file(RELATIVE_PATH rel_path ${PROJECT_SOURCE_DIR} ${GLSL})

    get_filename_component(FILE_NAME ${GLSL} NAME)

    set(SPIRV_OUT "${SHADER_ROOT_DIR}/${FILE_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV_OUT}
        COMMAND ${GLSL_VALIDATOR}
                -V
                --target-env ${VANTABLADE_VULKAN_TARGET}
                --spirv-val
                ${GLSL}
                -o ${SPIRV_OUT}
        DEPENDS ${GLSL}
        COMMENT "Compiling GLSL: ${rel_path} -> ${FILE_NAME}.spv"
        VERBATIM
    )

    list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUT})

    if (SPIRV_OPT)
        set(OPT_OUT "${SHADER_ROOT_DIR}/${FILE_NAME}.opt.spv")
        add_custom_command(
                OUTPUT ${OPT_OUT}
                COMMAND ${SPIRV_OPT} ${SPIRV_OUT} -O --canonicalize-ids --strip-debug --freeze-spec-const --eliminate-dead-const -o ${OPT_OUT}
                DEPENDS ${SPIRV_OUT}
                COMMENT "Optimizing SPIR-V: ${FILE_NAME}.spv -> ${FILE_NAME}.opt.spv"
                VERBATIM
        )
        list(APPEND SPIRV_BINARY_FILES ${OPT_OUT})
    endif ()
endforeach ()

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
        COMMENT "Building shader resources"
)

add_dependencies(vantablade_lib Shaders)

if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(vantablade_lib PUBLIC VANTABLADE_LIB_STATIC_DEFINE)
endif ()