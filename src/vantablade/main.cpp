#include <exception>
#include <fmt/base.h>
#include <fmt/format.h>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `Vantablade`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv) {
  const auto version = fmt::format("{} version {} git sha {}", Vantablade::cmake::project_name, Vantablade::cmake::project_version, Vantablade::cmake::git_short_sha);
  CLI::App app{version};
  try {
    app.set_version_flag("--version, -v", version);


    app.parse(argc, argv);

  } catch(const CLI::ParseError& e) {
    return app.exit(e);
  } catch(const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
  }
}
