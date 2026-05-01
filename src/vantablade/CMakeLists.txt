add_executable(vantablade main.cpp)

get_target_property(target_name vantablade NAME)
include("${CMAKE_SOURCE_DIR}/cmake/Simd.cmake")
set_simd_instructions(${target_name})

target_link_libraries(
        vantablade
        PRIVATE Vantablade::Vantablade_options
        Vantablade::Vantablade_warnings
        CLI11::CLI11
        Vantablade::vantablade_lib
)


Vantablade_configure_linker(vantablade)

target_include_directories(vantablade PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")