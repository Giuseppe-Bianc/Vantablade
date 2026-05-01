// NOLINTBEGIN(*-include-cleaner)
#include <CLI/CLI.hpp>
#include <Vantablade/vantablade.hpp>

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv) {
  const auto version = FORMAT("{} version {} git sha {}", Vantablade::cmake::project_name, Vantablade::cmake::project_version, Vantablade::cmake::git_short_sha);
  CLI::App app{version};
  try {
    app.set_version_flag("--version, -v", version);


    app.parse(argc, argv);

  } catch(const CLI::ParseError& e) {
    return app.exit(e);
  } catch(const std::exception &e) {
    // Handle any other types of exceptions
    LERROR("Unhandled exception in main: {}", e.what());
  }
}
// NOLINTEND(*-include-cleaner)