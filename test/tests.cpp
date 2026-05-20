// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-container-size-empty, *-move-const-arg, *-move-const-arg, *-pass-by-value, *-diagnostic-self-assign-overloaded, *-unused-using-decls, *-identifier-length, *-pro-bounds-constant-array-index, *-owning-memory, cert-err33-c, *-avoid-c-arrays, *-unsafe-functions, *-pro-bounds-array-to-pointer-decay, *-use-concise-preprocessor-directives, *-const-correctness)
// clang-format on
#include "testsConstants.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cstdio>
#include <future>
#include <memory>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>
#ifndef _WIN32
#include <unistd.h>
#endif

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::EndsWith;
using Catch::Matchers::Message;
using Catch::Matchers::StartsWith;

#define REQ_FORMAT(type, string) REQUIRE(FORMAT("{}", type) == (string));
#define REQ_FFORMAT(type, string) REQUIRE(FFORMAT("{}", type) == (string))
#define MSG_FORMAT(...) Message(FORMAT(__VA_ARGS__))
#define MSG_FFORMAT(...) Message(FORMAT(__VA_ARGS__))

static fs::path createTestFolderStructure() {
    fs::path testFolder = fs::temp_directory_path() / "test_folder_deletion";
    if(fs::exists(testFolder)) { fs::remove_all(testFolder); }

    fs::create_directories(testFolder / "subfolder1");
    fs::create_directories(testFolder / "subfolder2" / "nested");

    std::ofstream(testFolder / "file1.txt") << "File 1 content";
    std::ofstream(testFolder / "subfolder1" / "file2.txt") << "File 2 content";
    std::ofstream(testFolder / "subfolder2" / "nested" / "file3.txt") << "File 3 content";

    return testFolder;
}

namespace {
    // Helper function to create a file with content
    // NOLINTBEGIN(*-easily-swappable-parameters, *-signed-bitwise)
    void createFile(const std::string &infilename, const std::string &content) {
        std::ofstream ofs(infilename, std::ios::out | std::ios::binary);
        ofs << content;
        ofs.close();
    }
    // NOLINTEND(*-easily-swappable-parameters, *-signed-bitwise)

    // ─────────────────────────────────────────────────────────────
    // Helper: strip ANSI escape codes from a string for testing
    // ─────────────────────────────────────────────────────────────
    [[nodiscard]] std::string strip_ansi(std::string_view input) {
        std::string result;
        result.reserve(input.size());
        bool in_escape{false};
        for(const char c : input) {
            if(!in_escape) [[likely]] {
                if(c != '\x1b') [[likely]] {
                    result.push_back(c);
                } else [[unlikely]] {
                    in_escape = true;
                }
            } else [[unlikely]] {
                if(c == 'm' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) [[likely]] { in_escape = false; }
            }
        }

        return result;
    }

    // SAFETY: SavedFd wraps the duplicated (backup) file descriptor.
    struct SavedFd {
        int value;
    };

    // SAFETY: TargetFd wraps the descriptor whose slot is being restored-into.
    struct TargetFd {
        int value;
    };

    // ---------------------------------------------------------------------------
    // FdGuard — RAII wrapper that restores and closes a duplicated file descriptor.
    // Guarantees stdout restoration even when the captured callable throws.
    // ---------------------------------------------------------------------------
    struct FdGuard {
        const int saved_fd;
        const int target_fd;

        FdGuard(const SavedFd saved, const TargetFd target) noexcept : saved_fd{saved.value}, target_fd{target.value} {}

        // SAFETY: noexcept — destructor must never throw; dup2/close are C functions.
        ~FdGuard() noexcept {
#ifdef _WIN32
            (void)_dup2(saved_fd, target_fd);
            (void)_close(saved_fd);
#else
            (void)dup2(saved_fd, target_fd);
            (void)close(saved_fd);
#endif
        }

        FdGuard(const FdGuard &) = delete;
        FdGuard &operator=(const FdGuard &) = delete;
        FdGuard(FdGuard &&) = delete;
        FdGuard &operator=(FdGuard &&) = delete;
    };

    struct FileCloser {
        void operator()(FILE *file) const noexcept {
            if(file != nullptr) { (void)std::fclose(file); }
        }
    };

    // ─────────────────────────────────────────────────────────────
    // Helper: redirect stdout to a std::string for the duration of
    // a lambda, then restore it.
    // ─────────────────────────────────────────────────────────────
    struct CaptureStdout {
        CaptureStdout() = default;
        template <std::invocable Fn> [[nodiscard]] static std::string run(Fn &&fn) {
#ifdef _WIN32
            FILE *raw_tmp = nullptr;
            if(tmpfile_s(&raw_tmp) != 0 || raw_tmp == nullptr) { return {}; }
#else
            FILE *const raw_tmp = std::tmpfile();
            if(raw_tmp == nullptr) { return {}; }
#endif
            const std::unique_ptr<FILE, FileCloser> tmp{raw_tmp};

            (void)std::fflush(stdout);

#ifdef _WIN32
            const int stdout_fd = _fileno(stdout);
#else
            const int stdout_fd = fileno(stdout);
#endif
            if(stdout_fd < 0) { return {}; }

#ifdef _WIN32
            const int saved_fd = _dup(stdout_fd);
#else
            const int saved_fd = dup(stdout_fd);
#endif
            if(saved_fd < 0) { return {}; }

            const FdGuard fd_guard{SavedFd{saved_fd}, TargetFd{stdout_fd}};

#ifdef _WIN32
            (void)_dup2(_fileno(tmp.get()), stdout_fd);
#else
            (void)dup2(fileno(tmp.get()), stdout_fd);
#endif

            std::forward<Fn>(fn)();

            (void)std::fflush(stdout);

            (void)std::fseek(tmp.get(), 0L, SEEK_END);
            const long file_size = std::ftell(tmp.get());
            (void)std::fseek(tmp.get(), 0L, SEEK_SET);

            std::string result;
            if(file_size > 0L) { result.reserve(static_cast<std::string::size_type>(file_size)); }

            std::array<char, 4096> buf{};
            while(std::fgets(buf.data(), static_cast<int>(buf.size()), tmp.get()) != nullptr) {
                result.append(buf.data(), std::strlen(buf.data()));
            }

            return strip_ansi(result);
        }
    };

    // Helper: repeat a character n times.
    [[nodiscard]] std::string repeat_char(char c, std::size_t count) { return std::string(count, c); }

    // Helper: convert a string_view to a span<const std::byte> for low-level tests.
    [[nodiscard]] auto to_bytes(std::string_view sv) { return std::as_bytes(std::span{sv}); }

    // Keep Vulkan logging deterministic in the test process without touching the filesystem.
    [[maybe_unused]] const auto vulkan_log_bootstrap = [] {
        const auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        const auto logger = std::make_shared<spdlog::logger>("vulkan_tests", sink);
        logger->set_level(spdlog::level::trace);
        spdlog::set_default_logger(logger);
        return 0;
    }();
}  // namespace

TEST_CASE("my_error_handler(const std::string&) tests", "[error_handler]") {
    SECTION("Basic error handling") {
        const std::stringstream sss;
        auto *original = std::cerr.rdbuf(sss.rdbuf());  // Redirect cerr to stringstream
        my_error_handler("Sample error message");
        std::cerr.rdbuf(original);  // Restore cerr

        auto output = sss.str();
        REQUIRE_THAT(output, ContainsSubstring("Error occurred:"));
        REQUIRE_THAT(output, ContainsSubstring("Timestamp: "));
        REQUIRE_THAT(output, ContainsSubstring("Thread ID: "));
        REQUIRE_THAT(output, ContainsSubstring("Message:   Sample error message"));
    }

    SECTION("Error handler with different messages") {
        const std::stringstream sss;
        auto *original = std::cerr.rdbuf(sss.rdbuf());  // Redirect cerr to stringstream
        my_error_handler("Error 1");
        my_error_handler("Another error");
        std::cerr.rdbuf(original);  // Restore cerr

        auto output = sss.str();
        REQUIRE_THAT(output, ContainsSubstring("Message:   Error 1"));
        REQUIRE_THAT(output, ContainsSubstring("Message:   Another error"));
    }
}

TEST_CASE("TimeValues initialization", "[TimeValues]") {
    using vnd::TimeValues;

    SECTION("Default Constructor") {
        const TimeValues time;
        REQUIRE(time.get_seconds() == 0.0L);
        REQUIRE(time.get_millis() == 0.0L);
        REQUIRE(time.get_micro() == 0.0L);
        REQUIRE(time.get_nano() == 0.0L);
    }

    SECTION("Initialization with nanoseconds") {
        const TimeValues time(1'000'000.0L);  // 1 millisecond in nanoseconds
        REQUIRE(time.get_seconds() == 0.001L);
        REQUIRE(time.get_millis() == 1.0L);
        REQUIRE(time.get_micro() == 1000.0L);
        REQUIRE(time.get_nano() == 1'000'000.0L);
    }

    SECTION("Initialization with individual time units") {
        const TimeValues time(1.0L, 1000.0L, 1'000'000.0L, 1'000'000'000.0L);  // 1 second
        REQUIRE(time.get_seconds() == 1.0L);
        REQUIRE(time.get_millis() == 1000.0L);
        REQUIRE(time.get_micro() == 1'000'000.0L);
        REQUIRE(time.get_nano() == 1'000'000'000.0L);
    }
}

TEST_CASE("ValueLabel functionality", "[ValueLabel]") {
    using vnd::ValueLabel;

    SECTION("Transform time in microseconds") {
        const ValueLabel value(time_val_micro, "us");
        REQUIRE(value.transformTimeMicro(time_val_micro) == "1500us,0ns");

        const ValueLabel valueNonExact(time_val_micro2, "us");
        REQUIRE(valueNonExact.transformTimeMicro(time_val_micro2) == "1500us,500ns");
    }

    SECTION("Transform time in milliseconds") {
        const ValueLabel value(time_val_milli, "ms");
        REQUIRE(value.transformTimeMilli(time_val_milli) == "2ms,500us,0ns");

        const ValueLabel valueNonExact(time_val_milli2, "ms");
        REQUIRE(valueNonExact.transformTimeMilli(time_val_milli2) == "2ms,505us,0ns");
    }

    SECTION("Transform time in seconds") {
        const ValueLabel value(time_val_second, "s");
        REQUIRE(value.transformTimeSeconds(time_val_second) == "1s,0ms,0us,0ns");

        const ValueLabel valueNonExact(time_val_second2, "s");
        REQUIRE(valueNonExact.transformTimeSeconds(time_val_second2) == "1s,5ms,1us,0ns");
    }

    SECTION("ToString based on time label") {
        const ValueLabel secondsVal(2.0L, "s");
        REQUIRE(secondsVal.toString() == "2s,0ms,0us,0ns");

        const ValueLabel millisVal(2500.0L, "ms");
        REQUIRE(millisVal.toString() == "2500ms,0us,0ns");

        const ValueLabel microsVal(1500.0L, "us");
        REQUIRE(microsVal.toString() == "1500us,0ns");

        const ValueLabel unknownVal(3.0L, "unknown");
        REQUIRE(unknownVal.toString() == "3 unknown");
    }
}

TEST_CASE("Times functionality for  nano seconds", "[Times]") {
    const vnd::Times time(10.0L);  // 1 millisecond
    REQUIRE(time.getRelevantTimeframe().toString() == "10 ns");
}

TEST_CASE("Times functionality", "[Times]") {
    using vnd::Times;
    using vnd::TimeValues;
    using vnd::ValueLabel;

    SECTION("Initialization with nanoseconds") {
        const Times time(1'000'000.0L);  // 1 millisecond
        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "1000us,0ns");
    }

    SECTION("Initialization with TimeValues and custom labels") {
        const TimeValues timeVals(0.5L, 500.0L, 500'000.0L, 500'000'000.0L);  // 0.5 seconds
        const Times time(timeVals, "seconds", "milliseconds", "microseconds", "nanoseconds");

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "500 milliseconds");
    }

    SECTION("Switch between time units") {
        const TimeValues timeVals(0.001L, 1.0L, 1000.0L, 1'000'000.0L);  // 1 millisecond
        const Times time(timeVals);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "1000us,0ns");
    }

    SECTION("Very small nanoseconds") {
        const TimeValues timeVals(0.000001L, 0.001L, 1.0L, 1'000.0L);  // 1 microsecond
        const Times time(timeVals);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "1000 ns");
    }
}

TEST_CASE("Corner cases for TimeValues and Times", "[TimeValues]") {
    using vnd::Times;
    using vnd::TimeValues;
    using vnd::ValueLabel;

    SECTION("Negative values") {
        const TimeValues negativeTime(-1000000.0L);  // -1 millisecond
        const Times time(negativeTime);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
#ifdef __cpp_lib_format
        REQUIRE(relevantTime.toString() == "-1e+06 ns");
#else
        REQUIRE(relevantTime.toString() == "-1000000 ns");
#endif
    }
    SECTION("Zero values") {
        const TimeValues zeroTime(0.0L);  // Zero nanoseconds
        const Times time(zeroTime);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "0 ns");
    }

    SECTION("Large values") {
        const long double largeValue = 1'000'000'000'000.0L;  // 1 second in nanoseconds
        const TimeValues largeTime(largeValue);               // 1 second
        const Times time(largeTime);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "1000s,0ms,0us,0ns");
    }
}

TEST_CASE("get_current_timestamp() tests", "[timestamp]") {
    SECTION("Basic test") {
        auto timestamp = get_current_timestamp();
        REQUIRE(timestamp.size() >= timestampSize);
    }

    SECTION("Repeatability test") {
        auto timestamp1 = get_current_timestamp();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto timestamp2 = get_current_timestamp();
        REQUIRE(timestamp1 != timestamp2);
    }

    SECTION("Concurrency test") {
        constexpr int num_threads = 4;
        std::vector<std::future<std::string>> futures;
        for(int i = 0; i < num_threads; ++i) {
            // NOLINTNEXTLINE(*-inefficient-vector-operation)
            futures.emplace_back(std::async(std::launch::async, []() { return get_current_timestamp(); }));
        }
        for(auto &future : futures) {
            auto timestamp = future.get();
            REQUIRE(timestamp.size() >= timestampSize);
        }
    }
}

TEST_CASE("createFile: Successfully create a file with content", "[FileCreationResult]") {
    const fs::path testDir = fs::temp_directory_path() / "test_file_creation";
    fs::create_directories(testDir);

    const std::string fileName = "test_file.txt";
    std::stringstream content;
    content << "Hello, this is a test file.";

    auto result = vnd::FileCreationResult::createFile(testDir, fileName, content);

    const fs::path createdFilePath = testDir / fileName;
    REQUIRE(result.success());
    REQUIRE(fs::exists(createdFilePath));

    const std::string filecontent = vnd::readFromFile(createdFilePath.string());

    REQUIRE(filecontent == content.str());

    // Cleanup
    fs::remove_all(testDir);
}

TEST_CASE("createFile: Attempt to create a file in a non-existent directory", "[FileCreationResult]") {
    const fs::path nonExistentDir = fs::temp_directory_path() / "non_existent_directory";
    const std::string fileName = "test_file.txt";
    std::stringstream content;
    content << "Content for non-existent directory test.";

    const auto result = vnd::FileCreationResult::createFile(nonExistentDir, fileName, content);

    REQUIRE_FALSE(result.success());
    REQUIRE(!fs::exists(nonExistentDir / fileName));
}

TEST_CASE("createFile: Handle file creation when file already exists", "[FileCreationResult]") {
    const fs::path testDir = fs::temp_directory_path() / "test_file_creation_existing";
    fs::create_directories(testDir);

    const std::string fileName = "existing_file.txt";
    std::stringstream initialContent;
    initialContent << "Initial content.";

    const fs::path existingFilePath = testDir / fileName;
    std::ofstream outfile(existingFilePath);
    outfile << initialContent.rdbuf();
    outfile.close();

    REQUIRE(fs::exists(existingFilePath));

    std::stringstream newContent;
    newContent << "New content that overwrites.";

    auto result = vnd::FileCreationResult::createFile(testDir, fileName, newContent);

    REQUIRE(result.success());
    REQUIRE(fs::exists(existingFilePath));

    const std::string filecontent = vnd::readFromFile(existingFilePath.string());

    REQUIRE(filecontent == newContent.str());

    // Cleanup
    fs::remove_all(testDir);
}
TEST_CASE("createFile: Attempt to create a file with empty content", "[FileCreationResult]") {
    const fs::path testDir = fs::temp_directory_path() / "test_empty_content";
    fs::create_directories(testDir);

    const std::string fileName = "empty_content_file.txt";
    const std::stringstream emptyContent;

    auto result = vnd::FileCreationResult::createFile(testDir, fileName, emptyContent);

    const fs::path createdFilePath = testDir / fileName;
    REQUIRE(result.success());
    REQUIRE(fs::exists(createdFilePath));

    const std::string filecontent = vnd::readFromFile(createdFilePath.string());

    REQUIRE(filecontent.empty());

    // Cleanup
    fs::remove_all(testDir);
}

TEST_CASE("deleteFile: Successfully delete an existing file", "[FileDeletionResult]") {
    const fs::path testFile = fs::temp_directory_path() / "test_file_to_delete.txt";

    // Create the test file
    std::ofstream(testFile) << "Sample content for deletion test";
    REQUIRE(fs::exists(testFile));

    const auto result = vnd::FileDeletionResult::deleteFile(testFile);

    REQUIRE(result.success());
    REQUIRE(!fs::exists(testFile));
}

TEST_CASE("deleteFile: Attempt to delete a non-existent file", "[FileDeletionResult]") {
    const fs::path nonExistentFile = fs::temp_directory_path() / "non_existent_file.txt";

    REQUIRE(!fs::exists(nonExistentFile));

    const auto result = vnd::FileDeletionResult::deleteFile(nonExistentFile);

    REQUIRE_FALSE(result.success());
}

TEST_CASE("deleteFile: Attempt to delete a directory instead of a file", "[FileDeletionResult]") {
    const fs::path testDirectory = fs::temp_directory_path() / "test_directory";
    fs::create_directories(testDirectory);

    REQUIRE(fs::exists(testDirectory));
    REQUIRE(fs::is_directory(testDirectory));

    const auto result = vnd::FileDeletionResult::deleteFile(testDirectory);

    REQUIRE_FALSE(result.success());
    REQUIRE(fs::exists(testDirectory));  // Ensure the directory is not accidentally deleted

    // Cleanup
    fs::remove_all(testDirectory);
}

TEST_CASE("deleteFile: Handle exceptions gracefully", "[FileDeletionResult]") {
    const fs::path invalidPath;

    const auto result = vnd::FileDeletionResult::deleteFile(invalidPath);

    REQUIRE_FALSE(result.success());
}

TEST_CASE("deleteFolder: Successfully delete an existing folder structure", "[FolderDeletionResult]") {
    const fs::path testFolder = createTestFolderStructure();
    REQUIRE(fs::exists(testFolder));

    const auto result = vnd::FolderDeletionResult::deleteFolder(testFolder);

    REQUIRE(result.success());
    REQUIRE(!fs::exists(testFolder));
}

TEST_CASE("deleteFolder: Attempt to delete a non-existent folder", "[FolderDeletionResult]") {
    const fs::path nonExistentFolder = fs::temp_directory_path() / "non_existent_folder";
    REQUIRE(!fs::exists(nonExistentFolder));

    const auto result = vnd::FolderDeletionResult::deleteFolder(nonExistentFolder);

    REQUIRE_FALSE(result.success());
}

TEST_CASE("deleteFolder: Attempt to delete a file path instead of a folder", "[FolderDeletionResult]") {
    const fs::path testFile = fs::temp_directory_path() / "test_file.txt";

    // Create the test file
    std::ofstream(testFile) << "Test content";
    REQUIRE(fs::exists(testFile));

    const auto result = vnd::FolderDeletionResult::deleteFolder(testFile);

    REQUIRE_FALSE(result.success());
    REQUIRE(fs::exists(testFile));  // Ensure the file is not accidentally deleted

    // Cleanup
    fs::remove(testFile);
}

TEST_CASE("deleteFolder: Folder with nested subfolders and files", "[FolderDeletionResult]") {
    const fs::path testFolder = createTestFolderStructure();

    REQUIRE(fs::exists(testFolder));
    REQUIRE(fs::exists(testFolder / "subfolder1"));
    REQUIRE(fs::exists(testFolder / "subfolder2" / "nested" / "file3.txt"));

    auto result = vnd::FolderDeletionResult::deleteFolder(testFolder);

    REQUIRE(result.success());
    REQUIRE(!fs::exists(testFolder));
}

TEST_CASE("deleteFolder: Handle exceptions gracefully", "[FolderDeletionResult]") {
    const fs::path invalidPath;

    const auto result = vnd::FolderDeletionResult::deleteFolder(invalidPath);

    REQUIRE_FALSE(result.success());
}

TEST_CASE("std::filesystem::path formater", "[FMT]") { REQ_FFORMAT(std::filesystem::path("../ssss"), "../ssss"); }

TEST_CASE("Timer: MSTimes", "[timer]") {
    const auto timerNameData = timerName.data();
    vnd::Timer timer{timerNameData};
    std::this_thread::sleep_for(std::chrono::milliseconds(timerSleap));
    const std::string output = timer.to_string();
    const std::string new_output = (timer / timerCicles).to_string();
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerTime1.data()));
    REQUIRE_THAT(new_output, ContainsSubstring(timerTime2.data()));
}

TEST_CASE("Timer: MSTimes FMT", "[timer]") {
    const auto timerNameData = timerName.data();
    vnd::Timer timer{timerNameData};
    std::this_thread::sleep_for(std::chrono::milliseconds(timerSleap));
    const std::string output = FFORMAT("{}", timer);
    const std::string new_output = FFORMAT("{}", (timer / timerCicles));
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerTime1.data()));
    REQUIRE_THAT(new_output, ContainsSubstring(timerTime2.data()));
}

TEST_CASE("Timer: BigTimer", "[timer]") {
    const auto timerNameData = timerName.data();
    const vnd::Timer timer{timerNameData, vnd::Timer::Big};
    const std::string output = timer.to_string();
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerBigs.data()));
}

TEST_CASE("Timer: BigTimer FMT", "[timer]") {
    const auto timerNameData = timerName.data();
    vnd::Timer timer{timerNameData, vnd::Timer::Big};
    const std::string output = FFORMAT("{}", timer);
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerBigs.data()));
}

TEST_CASE("Timer: AutoTimer", "[timer]") {
    const vnd::Timer timer;
    const std::string output = timer.to_string();
    REQUIRE_THAT(output, ContainsSubstring("Timer"));
}

TEST_CASE("Timer: PrintTimer", "[timer]") {
    std::stringstream out;
    const vnd::Timer timer;
    out << timer;
    const std::string output = out.str();
    REQUIRE_THAT(output, ContainsSubstring(timerName2.data()));
}

TEST_CASE("Timer: PrintTimer FMT", "[timer]") {
    vnd::Timer timer;
    const std::string output = FFORMAT("{}", timer);
    REQUIRE_THAT(output, ContainsSubstring(timerName2.data()));
}

TEST_CASE("Timer: TimeItTimer", "[timer]") {
    vnd::Timer timer;
    const std::string output = timer.time_it([]() { std::this_thread::sleep_for(std::chrono::milliseconds(timerSleap2)); },
                                             timerResolution);
    REQUIRE_THAT(output, ContainsSubstring(timerTime1.data()));
}

TEST_CASE("FolderCreationResult Constructor", "[FolderCreationResult]") {
    SECTION("Default constructor") {
        const vnd::FolderCreationResult result;
        REQUIRE_FALSE(result.success());
        REQUIRE(result.path().value_or("").empty());
    }

    SECTION("Parameterized constructor") {
        const vnd::FolderCreationResult result(true, fs::path(testPaths));
        REQUIRE(result.success() == true);
        REQUIRE(result.path() == fs::path(testPaths));
    }
}

TEST_CASE("FolderCreationResult Setters", "[FolderCreationResult]") {
    vnd::FolderCreationResult result;

    SECTION("Set success") {
        result.set_success(true);
        REQUIRE(result.success() == true);
    }

    SECTION("Set path") {
        fs::path testPath(testPaths);
        REQUIRE(result.path().value_or("").empty());
        result.set_path(testPaths);
        REQUIRE(result.path() == testPath);
    }

    SECTION("Set path with empty string") {
        REQUIRE_THROWS_MATCHES(result.set_path(fs::path()), std::invalid_argument, Message("Path cannot be empty"));
    }
}

TEST_CASE("FolderCreationResult operator<< outputs correctly", "[FolderCreationResult]") {
    SECTION("Test with successful folder creation and valid path") {
        const fs::path folderPath = "/test/directory";
        const vnd::FolderCreationResult result(true, folderPath);

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: true, path_: /test/directory");
    }

    SECTION("Test with unsuccessful folder creation and no path") {
        const vnd::FolderCreationResult result(false, fs::path{});

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: false, path_: None");
    }

    SECTION("Test with successful folder creation but empty path") {
        const vnd::FolderCreationResult result(true, fs::path{});

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: true, path_: None");
    }

    SECTION("Test with unsuccessful folder creation and valid path") {
        const fs::path folderPath = "/another/test/directory";
        const vnd::FolderCreationResult result(false, folderPath);

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: false, path_: /another/test/directory");
    }

    SECTION("Test with default constructed FolderCreationResult") {
        const vnd::FolderCreationResult result;

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: false, path_: None");
    }
}

TEST_CASE("FolderCreationResult: Equality and Swap", "[FolderCreationResult]") {
    fs::path path1("/folder1");
    fs::path path2("/folder2");

    vnd::FolderCreationResult result1(true, path1);
    vnd::FolderCreationResult result2(false, path2);

    SECTION("Equality operator") {
        REQUIRE(result1 != result2);
        vnd::FolderCreationResult result3(true, path1);
        REQUIRE(result1 == result3);
    }

    SECTION("swap() function") {
        swap(result1, result2);
        REQUIRE(result1.success() == false);
        REQUIRE(result1.path().value() == path2);
        REQUIRE(result2.success() == true);
        REQUIRE(result2.path().value() == path1);
    }
}

TEST_CASE("FolderCreationResult Hash Value", "[FolderCreationResult]") {
    SECTION("Hash value is consistent for the same object") {
        const vnd::FolderCreationResult result(true, fs::path("/test/directory"));
        const std::size_t hash1 = hash_value(result);
        const std::size_t hash2 = hash_value(result);

        REQUIRE(hash1 == hash2);
    }

    SECTION("Hash value changes with different success status") {
        const vnd::FolderCreationResult result1(true, fs::path("/test/directory"));
        const vnd::FolderCreationResult result2(false, fs::path("/test/directory"));

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 != hash2);
    }

    SECTION("Hash value changes with different paths") {
        const vnd::FolderCreationResult result1(true, fs::path("/test/directory"));
        const vnd::FolderCreationResult result2(true, fs::path("/different/directory"));

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 != hash2);
    }

    SECTION("Identical objects have the same hash value") {
        const vnd::FolderCreationResult result1(true, fs::path("/test/directory"));
        const vnd::FolderCreationResult result2(true, fs::path("/test/directory"));

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 == hash2);
    }

    SECTION("Different objects have different hash values") {
        const vnd::FolderCreationResult result1(true, fs::path("/test/directory"));
        const vnd::FolderCreationResult result2(false, fs::path("/another/directory"));

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 != hash2);
    }

    SECTION("Hash for default constructed object is consistent") {
        const vnd::FolderCreationResult result1;
        const vnd::FolderCreationResult result2;

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 == hash2);
    }

    SECTION("Hash for default object vs object with empty path") {
        const vnd::FolderCreationResult result1;
        const vnd::FolderCreationResult result2(false, fs::path{});

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 == hash2);
    }
}

TEST_CASE("FolderCreationResult Folder Creation Functions", "[FolderCreationResult]") {
    // Create a temporary directory for testing
    auto tempDir = fs::temp_directory_path() / "vnd_test";
    const std::string folderName = "test_folder";
    const fs::path folderPath = tempDir / folderName;
    fs::create_directories(tempDir);

    SECTION("Create folder with valid parameters") {
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolder(folderName, tempDir);
        REQUIRE(result.success() == true);
        REQUIRE(result.path() == folderPath);
        [[maybe_unused]] auto unused = fs::remove_all(folderPath);
    }

    SECTION("Create folder with empty folder name") {
        const std::string emptyFolderName;
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolder(emptyFolderName, tempDir);
        REQUIRE_FALSE(result.success());
        REQUIRE(result.path()->empty());
    }

    SECTION("Create folder in non-existent parent directory") {
        const fs::path nonExistentParentDir = tempDir / "non_existent_dir";
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolder(folderName, nonExistentParentDir);
        REQUIRE(result.success() == true);
        REQUIRE(!result.path()->empty());
    }

    SECTION("Create folder in existing directory") {
        const fs::path nonExistentParentDir = tempDir / "non_existent_dir";
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolder(folderName, nonExistentParentDir);
        REQUIRE(result.success() == true);
        REQUIRE(!result.path()->empty());
        const std::string folderName2 = "test_folder";
        const vnd::FolderCreationResult result2 = vnd::FolderCreationResult::createFolder(folderName2, nonExistentParentDir);
        REQUIRE(result2.success() == true);
        REQUIRE(!result2.path()->empty());
    }

    SECTION("Create folder next to non-existent file") {
        const fs::path nonExistentFilePath = tempDir / "non_existent_file.txt";
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolderNextToFile(nonExistentFilePath, folderName);
        REQUIRE(result.success() == true);
        REQUIRE(!result.path()->empty());
        REQUIRE(!result.pathcref()->empty());
    }

    SECTION("Create folder next to existing file") {
        // Create a file in the temporary directory
        const fs::path filePathInner = tempDir / "test_file.txt";
        std::ofstream ofs(filePathInner);
        ofs.close();

        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolderNextToFile(filePathInner, folderName);
        REQUIRE(result.success() == true);
        REQUIRE(result.path() == folderPath);

        [[maybe_unused]] auto unused = fs::remove(filePathInner);
        [[maybe_unused]] auto unuseds = fs::remove_all(folderPath);
    }
    [[maybe_unused]] auto unused = fs::remove_all(tempDir);
}

TEST_CASE("vnd::readFromFile - Valid File", "[file]") {
    const std::string infilename = "testfile.txt";
    const std::string content = "This is a test.";

    createFile(infilename, content);

    auto result = vnd::readFromFile(infilename);
    REQUIRE(result == content);  // Ensure the content matches

    [[maybe_unused]] auto unsed = fs::remove(infilename);
}

TEST_CASE("vnd::readFromFile - Non-existent File", "[file]") {
    const std::string nonExistentFile = "nonexistent.txt";

    REQUIRE_THROWS_MATCHES(vnd::readFromFile(nonExistentFile), std::runtime_error, MSG_FORMAT("File not found: {}", nonExistentFile));
}

TEST_CASE("vnd::readFromFile - Non-regular File", "[file]") {
    const std::string dirName = "testdir";

    fs::create_directory(dirName);

    REQUIRE_THROWS_MATCHES(vnd::readFromFile(dirName), std::runtime_error, MSG_FORMAT("Path is not a regular file: {}", dirName));
    [[maybe_unused]] auto unsed = fs::remove(dirName);
}

TEST_CASE("vnd::readFromFile - Empty File", "[file]") {
    const std::string emtfilename = "emptyfile.txt";

    createFile(emtfilename, "");

    SECTION("Read from an empty file") {
        const auto result = vnd::readFromFile(emtfilename);
        REQUIRE(result.empty());  // Ensure the result is empty
    }

    [[maybe_unused]] auto unsed = fs::remove(emtfilename);
}

TEST_CASE("vnd::readFromFile - Large File", "[file]") {
    const std::string lrgfilename = "largefile.txt";
    const std::string largeContent(C_ST(1024 * 1024) * 10, 'a');  // 10 MB of 'a'

    createFile(lrgfilename, largeContent);

    SECTION("Read from a large file") {
        auto result = vnd::readFromFile(lrgfilename);
        REQUIRE(result == largeContent);  // Ensure content matches
    }

    [[maybe_unused]] auto unsed = fs::remove(lrgfilename);
}

TEST_CASE("GetBuildFolder - Standard Cases") {
    SECTION("Normal path without trailing slash") {
        const fs::path inputPath = fs::path("home/user/project").make_preferred();
        const fs::path expectedOutput = fs::path("home/user/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Path with trailing slash") {
        const fs::path inputPath = fs::path("home/user/project/").make_preferred();
        const fs::path expectedOutput = fs::path("home/user/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Nested directory structure") {
        const fs::path inputPath = fs::path("home/user/projects/client/app").make_preferred();
        const fs::path expectedOutput = fs::path("home/user/projects/client/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }
}

TEST_CASE("GetBuildFolder - Edge Cases") {
    SECTION("Root directory input") {
        const fs::path inputPath = fs::path("/").make_preferred();
        const fs::path expectedOutput = fs::path("/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Empty path") {
        const fs::path inputPath = fs::path("").make_preferred();
        const fs::path expectedOutput = fs::path(VANDIOR_BUILDFOLDER).make_preferred();  // No parent; expects vnbuild in current directory
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Relative path") {
        const fs::path inputPath = fs::path("folder/subfolder").make_preferred();
        const fs::path expectedOutput = fs::path("folder/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Single directory path") {
        const fs::path inputPath = fs::path("parent").make_preferred();
        const fs::path expectedOutput = fs::path(VANDIOR_BUILDFOLDER).make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Current directory input") {
        const fs::path inputPath = fs::path(".").make_preferred();
        const fs::path expectedOutput = fs::path(VANDIOR_BUILDFOLDER).make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Parent directory input") {
        const fs::path inputPath = fs::path("..").make_preferred();
        const fs::path expectedOutput = fs::path("../vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Path with special characters") {
        const fs::path inputPath = fs::path("/path/with special@chars!").make_preferred();
        const fs::path expectedOutput = fs::path("/path/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }
}

TEST_CASE("FormattedSize_StdFormat_ByteSuffix_FormatsTwoDecimalPlaces", "[FormattedSize]") {
    const FormattedSize fs{.value = 0.0L, .suffix = "B"};
    REQUIRE(std::format("{}", fs) == "0.00 B");
}

TEST_CASE("FormattedSize_StdFormat_OneByte_FormatsCorrectly", "[FormattedSize]") {
    const FormattedSize fs{.value = 1.0L, .suffix = "B"};
    REQUIRE(std::format("{}", fs) == "1.00 B");
}

TEST_CASE("FormattedSize_StdFormat_KBSuffix_FormatsCorrectly", "[FormattedSize]") {
    const FormattedSize fs{.value = 1.0L, .suffix = "KB"};
    REQUIRE(std::format("{}", fs) == "1.00 KB");
}

TEST_CASE("FormattedSize_StdFormat_MBSuffix_FormatsCorrectly", "[FormattedSize]") {
    const FormattedSize fs{.value = 1.0L, .suffix = "MB"};
    REQUIRE(std::format("{}", fs) == "1.00 MB");
}

TEST_CASE("FormattedSize_StdFormat_KiBSuffix_FormatsCorrectly", "[FormattedSize]") {
    const FormattedSize fs{.value = 1.0L, .suffix = "KiB"};
    REQUIRE(std::format("{}", fs) == "1.00 KiB");
}

TEST_CASE("FormattedSize_StdFormat_FractionalValue_FormatsWithTwoDecimals", "[FormattedSize]") {
    // 1.5 MB  → "1.50 MB"
    const FormattedSize fs{.value = 1.5L, .suffix = "MB"};
    REQUIRE(std::format("{}", fs) == "1.50 MB");
}

TEST_CASE("FormattedSize_StdFormat_LargeValue_FormatsCorrectly", "[FormattedSize]") {
    // 999.99 B
    const FormattedSize fs{.value = 999.99L, .suffix = "B"};
    REQUIRE(std::format("{}", fs) == "999.99 B");
}

TEST_CASE("FormattedSize_StdFormat_PBSuffix_FormatsCorrectly", "[FormattedSize]") {
    const FormattedSize fs{.value = 2.25L, .suffix = "PB"};
    REQUIRE(std::format("{}", fs) == "2.25 PB");
}

TEST_CASE("FormattedSize_StdFormat_InLargerString_EmbedsProperly", "[FormattedSize]") {
    const FormattedSize fs{.value = 1.0L, .suffix = "GB"};
    REQUIRE(std::format("Size: {}", fs) == "Size: 1.00 GB");
}

TEST_CASE("FormattedSize_FmtFormat_ByteSuffix_FormatsTwoDecimalPlaces", "[FormattedSize]") {
    const FormattedSize fs{.value = 0.0L, .suffix = "B"};
    REQUIRE(fmt::format("{}", fs) == "0.00 B");
}

TEST_CASE("FormattedSize_FmtFormat_KBSuffix_FormatsCorrectly", "[FormattedSize]") {
    const FormattedSize fs{.value = 1.0L, .suffix = "KB"};
    REQUIRE(fmt::format("{}", fs) == "1.00 KB");
}

TEST_CASE("FormattedSize_FmtFormat_KiBSuffix_FormatsCorrectly", "[FormattedSize]") {
    const FormattedSize fs{.value = 1.0L, .suffix = "KiB"};
    REQUIRE(fmt::format("{}", fs) == "1.00 KiB");
}

TEST_CASE("FormattedSize_FmtFormat_FractionalValue_FormatsWithTwoDecimals", "[FormattedSize]") {
    const FormattedSize fs{.value = 3.75L, .suffix = "GiB"};
    REQUIRE(fmt::format("{}", fs) == "3.75 GiB");
}

TEST_CASE("FormattedSize_FmtFormat_MatchesStdFormat_SameOutput", "[FormattedSize]") {
    const FormattedSize fs{.value = 512.0L, .suffix = "MiB"};
    REQUIRE(fmt::format("{}", fs) == std::format("{}", fs));
}

TEST_CASE("FormattedSizePair_StdFormat_ContainsSIAndIECValues", "[FormattedSizePair]") {
    const FormattedSizePair pair{.si = {.value = 1.0L, .suffix = "KB"}, .iec = {.value = 1.0L, .suffix = "KiB"}};
    const std::string result = std::format("{}", pair);
    REQUIRE_THAT(result, ContainsSubstring("1.00 KB"));
    REQUIRE_THAT(result, ContainsSubstring("1.00 KiB"));
}

TEST_CASE("FormattedSizePair_StdFormat_SIColumnIsLeftPaddedTo20", "[FormattedSizePair]") {
    const FormattedSizePair pair{.si = {.value = 1.0L, .suffix = "KB"}, .iec = {.value = 1.0L, .suffix = "KiB"}};
    const std::string result = std::format("{}", pair);
    // The entire string must be at least 41 chars (20 + 1 space + 20)
    REQUIRE(result.size() >= 41u);
    // The first 20 characters represent the SI column
    REQUIRE(result.substr(0, 7) == "1.00 KB");
}

TEST_CASE("FormattedSizePair_StdFormat_ZeroBytes_BothColumnsShowZeroB", "[FormattedSizePair]") {
    const FormattedSizePair pair{.si = {.value = 0.0L, .suffix = "B"}, .iec = {.value = 0.0L, .suffix = "B"}};
    const std::string result = std::format("{}", pair);
    REQUIRE_THAT(result, ContainsSubstring("0.00 B"));
}

TEST_CASE("FormattedSizePair_StdFormat_InLargerString_EmbedsProperly", "[FormattedSizePair]") {
    const FormattedSizePair pair{.si = {.value = 1.0L, .suffix = "MB"}, .iec = {.value = 1.0L, .suffix = "MiB"}};
    const std::string result = std::format("Pair: {}", pair);
    REQUIRE_THAT(result, StartsWith("Pair: "));
    REQUIRE_THAT(result, ContainsSubstring("1.00 MB"));
    REQUIRE_THAT(result, ContainsSubstring("1.00 MiB"));
}

TEST_CASE("FormattedSizePair_FmtFormat_ContainsSIAndIECValues", "[FormattedSizePair]") {
    const FormattedSizePair pair{.si = {.value = 1.0L, .suffix = "GB"}, .iec = {.value = 1.0L, .suffix = "GiB"}};
    const std::string result = fmt::format("{}", pair);
    REQUIRE_THAT(result, ContainsSubstring("1.00 GB"));
    REQUIRE_THAT(result, ContainsSubstring("1.00 GiB"));
}

TEST_CASE("FormattedSizePair_FmtFormat_MatchesStdFormat_SameOutput", "[FormattedSizePair]") {
    const FormattedSizePair pair{.si = {.value = 2.5L, .suffix = "TB"}, .iec = {.value = 2.27L, .suffix = "TiB"}};
    REQUIRE(fmt::format("{}", pair) == std::format("{}", pair));
}

TEST_CASE("FileSizeReport_StdFormat_ContainsByteCount", "[FileSizeReport]") {
    const FileSizeInfo info{1'000u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    REQUIRE_THAT(result, ContainsSubstring("Bytes : 1000"));
}

TEST_CASE("FileSizeReport_StdFormat_ContainsSIHeader", "[FileSizeReport]") {
    const FileSizeInfo info{1'024u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    REQUIRE_THAT(result, ContainsSubstring("SI"));
}

TEST_CASE("FileSizeReport_StdFormat_ContainsIECHeader", "[FileSizeReport]") {
    const FileSizeInfo info{1'024u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    REQUIRE_THAT(result, ContainsSubstring("IEC"));
}

TEST_CASE("FileSizeReport_StdFormat_ContainsDashedSeparators", "[FileSizeReport]") {
    const FileSizeInfo info{0u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    // Two separator rows of 41 dashes each
    REQUIRE_THAT(result, ContainsSubstring("-----------------------------------------"));
}

TEST_CASE("FileSizeReport_StdFormat_ZeroBytes_ContainsZeroB", "[FileSizeReport]") {
    const FileSizeInfo info{0u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    REQUIRE_THAT(result, ContainsSubstring("Bytes : 0"));
    REQUIRE_THAT(result, ContainsSubstring("0.00 B"));
}

TEST_CASE("FileSizeReport_StdFormat_1000Bytes_SIshowsKB", "[FileSizeReport]") {
    const FileSizeInfo info{1'000u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    REQUIRE_THAT(result, ContainsSubstring("1.00 KB"));
}

TEST_CASE("FileSizeReport_StdFormat_1000Bytes_IECshowsBytes", "[FileSizeReport]") {
    const FileSizeInfo info{1'000u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    // IEC keeps bytes: 1000.00 B
    REQUIRE_THAT(result, ContainsSubstring("1000.00 B"));
}

TEST_CASE("FileSizeReport_StdFormat_1024Bytes_IECshowsKiB", "[FileSizeReport]") {
    const FileSizeInfo info{1'024u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    REQUIRE_THAT(result, ContainsSubstring("1.00 KiB"));
}

TEST_CASE("FileSizeReport_StdFormat_OutputHasFourLines", "[FileSizeReport]") {
    const FileSizeInfo info{42u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    const auto newline_count = std::ranges::count(result, '\n');
    REQUIRE(newline_count == 4);
}

TEST_CASE("FileSizeReport_StdFormat_BytesLineIsFirst", "[FileSizeReport]") {
    const FileSizeInfo info{512u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = std::format("{}", report);
    REQUIRE_THAT(result, StartsWith("Bytes : 512"));
}

TEST_CASE("FileSizeReport_FmtFormat_ContainsByteCount", "[FileSizeReport]") {
    const FileSizeInfo info{2'048u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = fmt::format("{}", report);
    REQUIRE_THAT(result, ContainsSubstring("Bytes : 2048"));
}

TEST_CASE("FileSizeReport_FmtFormat_ContainsSIAndIECHeaders", "[FileSizeReport]") {
    const FileSizeInfo info{1'048'576u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = fmt::format("{}", report);
    REQUIRE_THAT(result, ContainsSubstring("SI"));
    REQUIRE_THAT(result, ContainsSubstring("IEC"));
}

TEST_CASE("FileSizeReport_FmtFormat_1MiB_ShowsCorrectSIandIEC", "[FileSizeReport]") {
    // 1 MiB = 1'048'576 bytes:  1.05 MB (SI)  |  1.00 MiB (IEC)
    const FileSizeInfo info{1'048'576u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const std::string result = fmt::format("{}", report);
    REQUIRE_THAT(result, ContainsSubstring("1.00 MiB"));
    REQUIRE_THAT(result, ContainsSubstring("MB"));
}

TEST_CASE("FileSizeReport_FmtFormat_MatchesStdFormat_SameOutput", "[FileSizeReport]") {
    const FileSizeInfo info{99'999u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    REQUIRE(fmt::format("{}", report) == std::format("{}", report));
}

TEST_CASE("FileSizeInfo_FormatThenStdFormat_EndToEndSI", "[FileSizeInfo]") {
    constexpr FileSizeInfo info{1'000'000u};
    const FormattedSize fs = info.format(kSI);
    REQUIRE(std::format("{}", fs) == "1.00 MB");
}

TEST_CASE("FileSizeInfo_FormatThenStdFormat_EndToEndIEC", "[FileSizeInfo]") {
    constexpr FileSizeInfo info{1'048'576u};
    const FormattedSize fs = info.format(kIEC);
    REQUIRE(std::format("{}", fs) == "1.00 MiB");
}

TEST_CASE("FileSizeReport_MakePairThenStdFormat_EndToEndPair", "[FileSizeReport]") {
    const FileSizeInfo info{1'000'000u};
    const FileSizeReport report{.info = info, .si_sys = kSI, .iec_sys = kIEC};
    const FormattedSizePair pair = report.make_pair();
    const std::string result = std::format("{}", pair);
    REQUIRE_THAT(result, ContainsSubstring("1.00 MB"));
}

TEST_CASE("FileSizeInfo_AllSIPrefixLevels_FormatCorrectly", "[FileSizeInfo]") {
    // Validates that every SI prefix level formats without crash and includes
    // the expected suffix.
    struct Case {
        uintmax_t bytes;
        std::string_view suffix;
    };
    const std::array<Case, 6> cases{{
        {.bytes = 0u, .suffix = "B"},
        {.bytes = 1000u, .suffix = "KB"},
        {.bytes = 1000000u, .suffix = "MB"},
        {.bytes = 1000000000u, .suffix = "GB"},
        {.bytes = 1000000000000u, .suffix = "TB"},
        {.bytes = 1000000000000000u, .suffix = "PB"},
    }};

    for(const auto &[bytes, expected_suffix] : cases) {
        const FileSizeInfo info{bytes};
        const FormattedSize fs = info.format(kSI);
        INFO("bytes = " << bytes);
        REQUIRE(fs.suffix == expected_suffix);
        const std::string formatted = std::format("{}", fs);
        REQUIRE_THAT(formatted, EndsWith(std::string(expected_suffix)));
    }
}

TEST_CASE("FileSizeInfo_AllIECPrefixLevels_FormatCorrectly", "[FileSizeInfo]") {
    struct Case {
        uintmax_t bytes;
        std::string_view suffix;
    };
    const std::array<Case, 6> cases{{
        {.bytes = 0u, .suffix = "B"},
        {.bytes = 1024u, .suffix = "KiB"},
        {.bytes = C_UIMT(1024) * 1024u, .suffix = "MiB"},
        {.bytes = C_UIMT(1024) * 1024u * 1024u, .suffix = "GiB"},
        {.bytes = C_UIMT(1024) * 1024u * 1024u * 1024u, .suffix = "TiB"},
        {.bytes = C_UIMT(1024) * 1024u * 1024u * 1024u * 1024u, .suffix = "PiB"},
    }};

    for(const auto &[bytes, expected_suffix] : cases) {
        const FileSizeInfo info{bytes};
        const FormattedSize fs = info.format(kIEC);
        INFO("bytes = " << bytes);
        REQUIRE(fs.suffix == expected_suffix);
        const std::string formatted = std::format("{}", fs);
        REQUIRE_THAT(formatted, EndsWith(std::string(expected_suffix)));
    }

    // Zero bytes: stays at "B" with value 0.0, not 1.0
    SECTION("ZeroBytes_SuffixIsBAndValueIsZero") {
        const FileSizeInfo info{0u};
        const FormattedSize fs = info.format(kIEC);
        REQUIRE(fs.suffix == "B");
        REQUIRE(fs.value == 0.0L);
        REQUIRE(std::format("{}", fs) == "0.00 B");
    }
}

TEST_CASE("Vulkan object strings stay stable", "[vulkan][strings]") {
    REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_UNKNOWN)} == "UNKNOWN");
    REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_INSTANCE)} == "INSTANCE");
    REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_PIPELINE_LAYOUT)} == "PIPELINE_LAYOUT");
    REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_SWAPCHAIN_KHR)} == "SWAPCHAIN_KHR");
    REQUIRE(std::string_view{VkObjectString(static_cast<VkObjectType>(-1))} == "UNHANDLED");
}

TEST_CASE("Vulkan flag string helpers preserve ordering and empty inputs", "[vulkan][flags]") {
    SECTION("VkMemoryPropertyFlagsString") {
        REQUIRE(VkMemoryPropertyFlagsString(0) == "");
        REQUIRE(VkMemoryPropertyFlagsString(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) ==
                "DEVICE_LOCAL | HOST_VISIBLE");
    }

    SECTION("VkQueueFlagsString") {
        REQUIRE(VkQueueFlagsString(0) == "");
        REQUIRE(VkQueueFlagsString(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) ==
                "GRAPHICS | COMPUTE | TRANSFER");
    }

    SECTION("VkDebugUtilsMessageTypeFlagsEXTString") {
        REQUIRE(VkDebugUtilsMessageTypeFlagsEXTString(0) == "");
        REQUIRE(VkDebugUtilsMessageTypeFlagsEXTString(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) == "[GENERAL] ");
        REQUIRE_THAT(VkDebugUtilsMessageTypeFlagsEXTString(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT),
                     ContainsSubstring("[GENERAL]"));
        REQUIRE_THAT(VkDebugUtilsMessageTypeFlagsEXTString(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT),
                     ContainsSubstring("[PERFORMANCE]"));
    }
}

TEST_CASE("VK_CHECK reports Vulkan failures with diagnostics", "[vulkan][check]") {
    SECTION("VK_CHECK accepts VK_SUCCESS") {
        REQUIRE_NOTHROW(([&] { VK_CHECK(VK_SUCCESS, "VK_CHECK should not throw for success"); }()));
    }

    SECTION("VK_CHECK throws on failure") {
        const auto fail = [] { VK_CHECK(VK_ERROR_INITIALIZATION_FAILED, "device bootstrap failed"); };

        REQUIRE_THROWS_WITH(fail(), ContainsSubstring("device bootstrap failed"));
        REQUIRE_THROWS_WITH(fail(), ContainsSubstring("VK_ERROR_INITIALIZATION_FAILED"));
    }
}

TEST_CASE("VK_CHECK_SYNC_OBJECTS requires every call to succeed", "[vulkan][check]") {
    SECTION("VK_CHECK_SYNC_OBJECTS accepts three successes") {
        REQUIRE_NOTHROW(([&] { VK_CHECK_SYNC_OBJECTS(VK_SUCCESS, VK_SUCCESS, VK_SUCCESS, "sync objects should not throw"); }()));
    }

    SECTION("VK_CHECK_SYNC_OBJECTS reports the failing result") {
        const auto fail = [] { VK_CHECK_SYNC_OBJECTS(VK_SUCCESS, VK_TIMEOUT, VK_SUCCESS, "sync object wait failed"); };

        REQUIRE_THROWS_WITH(fail(), ContainsSubstring("sync object wait failed"));
        REQUIRE_THROWS_WITH(fail(), ContainsSubstring("VK_TIMEOUT"));
    }
}

TEST_CASE("VK_CHECK_SWAPCHAIN accepts only success or suboptimal", "[vulkan][check]") {
    SECTION("VK_CHECK_SWAPCHAIN accepts VK_SUCCESS") {
        REQUIRE_NOTHROW(([&] { VK_CHECK_SWAPCHAIN(VK_SUCCESS, "swapchain should not throw"); }()));
    }

    SECTION("VK_CHECK_SWAPCHAIN accepts VK_SUBOPTIMAL_KHR") {
        REQUIRE_NOTHROW(([&] { VK_CHECK_SWAPCHAIN(VK_SUBOPTIMAL_KHR, "swapchain should not throw"); }()));
    }

    SECTION("VK_CHECK_SWAPCHAIN throws on hard failure") {
        const auto fail = [] { VK_CHECK_SWAPCHAIN(VK_ERROR_OUT_OF_DATE_KHR, "swapchain out of date"); };

        REQUIRE_THROWS_WITH(fail(), ContainsSubstring("swapchain out of date"));
    }
}

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-container-size-empty, *-move-const-arg, *-move-const-arg, *-pass-by-value, *-diagnostic-self-assign-overloaded, *-unused-using-decls, *-identifier-length, *-pro-bounds-constant-array-index, *-owning-memory, cert-err33-c, *-avoid-c-arrays, *-unsafe-functions, *-pro-bounds-array-to-pointer-decay, *-use-concise-preprocessor-directives, *-const-correctness)
// clang-format on
