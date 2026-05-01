include(GenerateExportHeader)


add_library(vantablade_core_lib vantabladeCore.cpp)


add_library(Vantablade::vantablade_core_lib ALIAS vantablade_core_lib)

get_target_property(target_name vantablade_core_lib NAME)
include("${CMAKE_SOURCE_DIR}/cmake/Simd.cmake")
set_simd_instructions(${target_name})

target_link_libraries(vantablade_core_lib
        PRIVATE
        Vantablade_options
        Vantablade_warnings
        PUBLIC
        fmt::fmt
        spdlog::spdlog)

target_include_directories(vantablade_core_lib ${WARNING_GUARD}
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/configured_files/include>
)

if (Vantablade_BUILD_FUZZ_TESTS)
    target_link_libraries(vantablade_core_lib PRIVATE -fsanitize=fuzzer-no-link)
    target_compile_options(vantablade_core_lib PRIVATE -fsanitize=fuzzer-no-link)
endif ()

target_compile_features(vantablade_core_lib PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

Vantablade_configure_linker(vantablade_core_lib)

set_target_properties(
        vantablade_core_lib
        PROPERTIES VERSION ${PROJECT_VERSION}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES)

generate_export_header(vantablade_core_lib EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/VantabladeCore/vantablade_core_export.hpp)

if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(vantablade_core_lib PUBLIC VANTABLADE_CORE_LIB_STATIC_DEFINE)
endif ()