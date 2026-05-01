// NOLINTBEGIN(*-include-cleaner)
#include <Vantablade/vantablade.hpp>
#ifdef _WIN32
#include <windows.h>
#endif
DISABLE_WARNINGS_PUSH(
    4005 4201 4459 4514 4625 4626 4820 6244 6285 6385 6386 26408 26409 26415 26418 26426 26429 26432 26437 26438 26440 26446 26447 26450 26451 26455 26457 26459 26460 26461 26462 26467 26472 26473 26474 26475 26481 26482 26485 26490 26491 26493 26494 26495 26496 26497 26498 26800 26814 26818 26821 26826 26827)
#include <CLI/CLI.hpp>
DISABLE_WARNINGS_POP()

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv) {
    INIT_LOG();
    const vnd::AutoTimer compilationTime("Total Execution");
#ifdef _WIN32
    const vnd::Timer winConsoleTimer("Windows console setup");
    // Set UTF-8 code page for Windows console
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // Enable virtual terminal processing (ANSI escape codes) on both
    // stdout and stderr — each handle requires its own SetConsoleMode call.
    for(const DWORD handle_id : {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE}) {
        // NOLINTNEXTLINE(*-identifier-length)
        if(HANDLE h = GetStdHandle(handle_id); h != INVALID_HANDLE_VALUE && h != nullptr) {
            DWORD dwMode = 0;
            if(GetConsoleMode(h, &dwMode) != 0) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(h, dwMode);  // non-fatal if this fails
            }
        }
    }
    LINFO("{}", winConsoleTimer);
#endif
    const auto version = FORMAT("{} version {} git sha {}", Vantablade::cmake::project_name, Vantablade::cmake::project_version,
                                Vantablade::cmake::git_short_sha);
    CLI::App app{version};
    try {
        app.set_version_flag("--version, -v", version);
        app.parse(argc, argv);

        Window lveWindow{800, 600, "Vulkan GLFW"};
        while(!lveWindow.shouldClose()) [[likely]] {
            glfwPollEvents();
        }
    } catch(const CLI::ParseError &e) { return app.exit(e); } catch(const std::exception &e) {
        // Handle any other types of exceptions
        LERROR("Unhandled exception in main: {}", e.what());
    }
}
// NOLINTEND(*-include-cleaner)