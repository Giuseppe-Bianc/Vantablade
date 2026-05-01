add_executable(vantablade main.cpp)

target_link_libraries(
        vantablade
  PRIVATE Vantablade::Vantablade_options
          Vantablade::Vantablade_warnings
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog)


Vantablade_configure_linker(vantablade)

target_include_directories(vantablade PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")