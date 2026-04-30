add_executable(intro main.cpp)

target_link_libraries(
  intro
  PRIVATE Vantablade::Vantablade_options
          Vantablade::Vantablade_warnings
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog
          lefticus::tools
          ftxui::screen
          ftxui::dom
          ftxui::component)


Vantablade_configure_linker(intro)

target_include_directories(intro PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")