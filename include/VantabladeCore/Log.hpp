/**
 * @file Log.hpp
 * @brief Logging via SPDLOG with selectable sync/async backend.
 *
 * @details Two setup functions are provided:
 *
 *   setup_logger()        – original synchronous path (default for INIT_LOG).
 *   setup_logger_async()  – async path: messages are enqueued and written on a
 *                           dedicated background thread (use INIT_LOG_ASYNC).
 *
 *   Both expose the identical macro interface so call sites need zero changes
 *   when switching between the two backends.
 *
 * ### C++23 / spdlog community guidelines applied
 *
 *   - my_error_handler is noexcept: spdlog calls it from internal contexts
 *     that cannot tolerate exceptions.
 *   - flush_on(spdlog::level::err): both loggers flush immediately on every
 *     error or critical message, preventing loss on crash.
 *   - register_logger is called BEFORE set_default_logger: set_default_logger
 *     inserts the logger into the registry map itself; a subsequent
 *     register_logger call reaches throw_if_exists_ and throws. The correct
 *     order is register first, then set as default.
 *   - atexit handler (async only) calls drop_all() then shutdown(): drop_all
 *     releases all logger shared_ptrs from the registry so the thread pool is
 *     idle when shutdown() joins the worker thread.
 *   - flush_every (async only, commented): spdlog recommends periodic flushing
 *     for low-severity messages that never reach the flush_on threshold.
 *   - static inline constexpr: 'static' forces internal linkage and makes
 *     'inline' a no-op. Kept for consistency with the rest of this codebase;
 *     pure C++17/23 style would use 'inline constexpr' (external linkage) or
 *     plain 'constexpr' (internal linkage).
 *
 * @defgroup Logging Macros
 * @brief Macros for logging messages at various levels.
 * @{
 *
 * @section Overview
 * This module provides a set of macros for logging messages at different severity levels.
 * The macros wrap around the corresponding functions provided by the SPDLOG library,
 * making it easy to integrate logging into your application.
 *
 * @section Logging Levels
 * - LTRACE: Logs trace messages, which are the most detailed and typically used for debugging.
 * - LDEBUG: Logs debug messages, useful during development and testing.
 * - LINFO: Logs informational messages about the application's state.
 * - LWARN: Logs warning messages that indicate potential issues.
 * - LERROR: Logs error messages for non-critical errors.
 * - LCRITICAL: Logs critical messages for severe errors requiring immediate attention.
 *
 * @section Initialization
 * The logging system must be initialized before any logging can occur.
 * Use the INIT_LOG() macro to set up the logging configuration with default settings.
 * This includes setting a default pattern for log messages and creating a console logger.
 *
 * @par Example:
 * @code{.cpp}
 * INIT_LOG();
 * LINFO("Logging system initialized.");
 * INIT_LOG_ASYNC();
 * LINFO("Logging system initialized with async backend.");
 * @endcode
 * }.
 */
#pragma once
// NOLINTBEGIN(*-include-cleaner)

// clang-format off
#include "disableWarn.hpp"
#include <iostream>
#include <chrono>
#include "format.hpp"
// clang-format on

namespace ch = std::chrono;
/** \cond */
DISABLE_WARNINGS_PUSH(
    4005 4201 4459 4514 4625 4626 4820 6244 6285 6385 6386 26409 26415 26418 26429 26432 26437 26438 26440 26446 26447 26450 26451 26455 26457 26459 26460 26461 26467 26472 26473 26474 26475 26481 26482 26485 26490 26491 26493 26494 26495 26496 26497 26498 26800 26814 26818 26826)
/** \endcond */

/**
 * @brief Configures SPDLOG to accept all severity levels from TRACE and above.
 *
 * Must be defined before the spdlog headers are included.
 */
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

DISABLE_CLANG_WARNINGS_PUSH("-Wunused-result")
#include <spdlog/async.h>  // async_logger, init_thread_pool, thread_pool
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>   // <-- add this
#include <spdlog/spdlog.h>
DISABLE_CLANG_WARNINGS_POP()

DISABLE_WARNINGS_POP()

/** \cond */
#ifndef _MSC_VER
/**
 * @brief Disable GCC warning about uninitialized variables for spdlog.
 */
DISABLE_GCC_WARNINGS_PUSH("-Wuninitialized")
#include <spdlog/details/null_mutex.h>
DISABLE_GCC_WARNINGS_POP()
#endif
/** \endcond */

// ---------------------------------------------------------------------------
// Async configuration constants
//
// NOTE: 'static inline constexpr' at namespace scope – 'static' forces
// internal linkage, which makes 'inline' a no-op.  Pure C++17/23 would use
// 'inline constexpr' (external linkage, one shared definition) or plain
// 'constexpr' (internal linkage, fine for compile-time constants).  The
// 'static inline constexpr' form is kept here for consistency with the rest
// of this codebase (see headersCore.hpp).
// ---------------------------------------------------------------------------

/// Log file path.
static inline constexpr std::string_view LOG_FILE_PATH = "logs/app.log";

/// Max size per log file before rotation (10 MiB).
static inline constexpr std::size_t LOG_FILE_MAX_SIZE = 10u * 1024u * 1024u;

/// Number of rotated files to keep on disk.
static inline constexpr std::size_t LOG_FILE_MAX_FILES = 5u;

/// Lock-free async queue depth.  Must be a power of two.
/// 8 192 slots × ~32 B each ≈ 256 KiB; handles any renderer burst comfortably.
static inline constexpr std::size_t ASYNC_QUEUE_SIZE = 8192u;

/// Dedicated logging threads.  One thread eliminates sink-mutex contention
/// while keeping I/O off the render/main thread.  Increase to 2 only if
/// multiple heavy file sinks are added.
static inline constexpr std::size_t ASYNC_THREAD_COUNT = 1u;

/**
 * @brief Macro for logging trace messages using SPDLOG_TRACE.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log detailed tracing information for debugging purposes.
 * This macro is a wrapper around the SPDLOG_TRACE macro provided by the spdlog library.
 * Trace messages are the most verbose and are typically enabled only during intensive debugging.
 *
 * @par Example:
 * @code{.cpp}
 * LTRACE("Entering function {} with parameter {}", funcName, param);
 * @endcode
 *
 * @see spdlog::trace
 */
#define LTRACE(...) SPDLOG_TRACE(__VA_ARGS__)

/**
 * @brief Macro for logging debug messages using SPDLOG_DEBUG.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log debug information helpful during development and testing.
 * This macro is a wrapper around the SPDLOG_DEBUG macro provided by the spdlog library.
 * Debug messages provide more detail than info messages but less than trace messages.
 *
 * @par Example:
 * @code{.cpp}
 * LDEBUG("Processing item {} of {}", currentIndex, totalCount);
 * @endcode
 *
 * @see spdlog::debug
 */
#define LDEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)

/**
 * @brief Macro for logging informational messages using SPDLOG_INFO.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log general information about the application's state.
 * This macro is a wrapper around the SPDLOG_INFO macro provided by the spdlog library.
 * Info messages represent normal operational events that don't require special attention.
 *
 * @par Example:
 * @code{.cpp}
 * LINFO("Application started successfully");
 * @endcode
 *
 * @see spdlog::info
 */
#define LINFO(...) SPDLOG_INFO(__VA_ARGS__)

/**
 * @brief Macro for logging warning messages using SPDLOG_WARN.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log non-critical warnings that might indicate potential issues.
 * This macro is a wrapper around the SPDLOG_WARN macro provided by the spdlog library.
 * Warning messages indicate situations that may become problematic if not addressed.
 *
 * @par Example:
 * @code{.cpp}
 * LWARN("Configuration file not found, using defaults");
 * @endcode
 *
 * @see spdlog::warn
 */
#define LWARN(...) SPDLOG_WARN(__VA_ARGS__)

/**
 * @brief Macro for logging error messages using SPDLOG_ERROR.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log errors that do not prevent the application from continuing.
 * This macro is a wrapper around the SPDLOG_ERROR macro provided by the spdlog library.
 * Error messages indicate problems that need attention but don't halt execution.
 *
 * @par Example:
 * @code{.cpp}
 * LERROR("Failed to connect to database: {}", errorMessage);
 * @endcode
 *
 * @see spdlog::error
 */
#define LERROR(...) SPDLOG_ERROR(__VA_ARGS__)

/**
 * @brief Macro for logging critical messages using SPDLOG_CRITICAL.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log critical errors that require immediate attention.
 * This macro is a wrapper around the SPDLOG_CRITICAL macro provided by the spdlog library.
 * Critical messages indicate severe errors that may prevent the application from continuing.
 *
 * @par Example:
 * @code{.cpp}
 * LCRITICAL("Out of memory, application cannot continue");
 * @endcode
 *
 * @see spdlog::critical
 */
#define LCRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

// ---------------------------------------------------------------------------
// Timestamp helper
// ---------------------------------------------------------------------------

/**
 * @brief Returns the current wall-clock time formatted to millisecond precision.
 *
 * @return std::string "YYYY-MM-DD HH:MM:SS.mmm".
 *
 * @details This function retrieves the current system time and formats it
 *         as a string with millisecond precision. It is used internally
 *         by the error handler to timestamp error messages.
 *
 * @note The function uses std::chrono for time retrieval and formatting.
 *
 * @par Example:
 * @code{.cpp}
 * std::string timestamp = get_current_timestamp();
 * // Output: "2024-01-15 14:30:45.123"
 * @endcode
 */
[[nodiscard]] inline std::string get_current_timestamp() {
    const auto now = ch::floor<ch::milliseconds>(ch::system_clock::now());
    return FORMAT("{:%Y-%m-%d %H:%M:%S}", now);
}

// clang-format off
/**
 * @brief Custom handler for spdlog-internal errors.
 *
 * @param[in] msg The diagnostic message produced by spdlog.
 *
 * @details Writes a timestamped, thread-tagged error record to std::cerr.
 *          The function is declared noexcept because spdlog invokes it from
 *          internal contexts that cannot tolerate exceptions; a throwing
 *          handler would cause std::terminate.
 *
 * @note Install this handler before any logger is created:
 *       spdlog::set_error_handler(my_error_handler);
 */
inline void my_error_handler(const std::string& msg) noexcept {try {
        std::cerr <<
            FORMAT("Error occurred:\n  Timestamp: {}\n  Thread ID: {}\n  Message:   {}\n  Note: Error originated within spdlog internals.\n",
            get_current_timestamp(),
            std::this_thread::get_id(),
            msg);
    } catch (...) {}  // NOLINT(*-empty-catch) — must not propagate out of noexcept boundary
}
// clang-format on

/**
 * @brief Configures the default spdlog logger in synchronous mode.
 *
 * @details Creates a multi-threaded logger backed by a coloured stdout sink.
 *          Every log call writes to the sink on the calling thread.  Zero
 *          queue overhead; suitable for development builds or tools where
 *          logging latency is not a concern.
 *
 *          ### spdlog registry note
 *          spdlog::set_default_logger internally inserts the logger into the
 *          registry map (loggers_[name] = logger).  Calling register_logger
 *          AFTER set_default_logger would reach throw_if_exists_ and throw
 *          because "main" is already in the map.  This function therefore
 *          calls register_logger FIRST, then set_default_logger, matching the
 *          safe order documented in the spdlog source.
 *
 *          ### Flush policy
 *          flush_on(spdlog::level::err) ensures the sink is flushed
 *          immediately on every error or critical message, preventing loss
 *          if the process terminates abnormally.
 *
 * @throws spdlog::spdlog_ex if logger creation fails.
 *
 * @see setup_logger_async, INIT_LOG
 *
 * @par Example:
 * @code{.cpp}
 * setup_logger();
 * LINFO("Logger configured (sync)");
 * @endcode
 */
inline void setup_logger() {
    std::filesystem::create_directories("logs");
    std::vector<spdlog::sink_ptr> sinks;
    sinks.reserve(3);  // PERF: 2 sinks declared; avoids reallocation

    const auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdout_sink->set_level(spdlog::level::trace);
    stdout_sink->set_pattern(R"(%^[%T %l] %v%$)");

    // Stderr sink available when split output is desired:
    // const auto stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    // stderr_sink->set_level(spdlog::level::warn);
    // sinks.push_back(stderr_sink);
    const auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(std::string(LOG_FILE_PATH), LOG_FILE_MAX_SIZE,LOG_FILE_MAX_FILES);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern(R"([%Y-%m-%d %T.%e] [%l] [%t] [%s:%#] %v)");
    sinks.push_back(file_sink);

    sinks.push_back(stdout_sink);

    const auto logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);

    // Flush immediately on error or critical.
    logger->flush_on(spdlog::level::trace);

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

// ---------------------------------------------------------------------------
// setup_logger_async  (async backend)
// ---------------------------------------------------------------------------

/**
 * @brief Configures the default spdlog logger in asynchronous mode.
 *
 * @details Initialises a thread pool (ASYNC_QUEUE_SIZE slots,
 *          ASYNC_THREAD_COUNT worker threads) and binds an async_logger to it.
 *          The calling thread enqueues log records and returns immediately;
 *          the worker thread drains the queue and writes to sinks independently.
 *
 *          ### Overflow policy: block
 *          async_overflow_policy::block parks the calling thread only when the
 *          queue is completely full, providing backpressure without silently
 *          discarding or overwriting messages (overrun_oldest).
 *
 *          ### Flush policy
 *          flush_on(spdlog::level::err) causes the worker to flush the sink
 *          immediately after writing any error or critical record.
 *
 *          ### Shutdown
 *          A std::atexit handler calls spdlog::drop_all() then
 *          spdlog::shutdown().  drop_all releases all logger shared_ptrs from
 *          the registry so the thread pool is idle (queue empty) when
 *          shutdown() joins the worker thread.  This sequence prevents races
 *          between in-flight log records and static-destructor ordering.
 *
 *          ### spdlog registry note
 *          Same ordering constraint as setup_logger: register_logger is called
 *          before set_default_logger.
 *
 * @pre spdlog::set_error_handler must be installed before this call so that
 *      thread-pool errors are routed to my_error_handler.
 *
 * @throws spdlog::spdlog_ex if thread-pool or logger creation fails.
 *
 * @see setup_logger, INIT_LOG_ASYNC
 *
 * @par Example:
 * @code{.cpp}
 * setup_logger_async();
 * LINFO("Logger configured (async)");
 * @endcode
 */
inline void setup_logger_async() {
    // One thread pool per process.  Must be created before the async_logger.
    spdlog::init_thread_pool(ASYNC_QUEUE_SIZE, ASYNC_THREAD_COUNT);
    std::filesystem::create_directories("logs");

    std::vector<spdlog::sink_ptr> sinks;
    sinks.reserve(3);

    const auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdout_sink->set_level(spdlog::level::trace);
    stdout_sink->set_pattern(R"(%^[%T %l] %v%$)");

    // const auto stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    // stderr_sink->set_level(spdlog::level::warn);
    // sinks.push_back(stderr_sink);
    const auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(std::string(LOG_FILE_PATH), LOG_FILE_MAX_SIZE,LOG_FILE_MAX_FILES);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern(R"([%Y-%m-%d %T.%e] [%l] [%t] [%s:%#] %v)");
    sinks.push_back(file_sink);

    sinks.push_back(stdout_sink);

    const auto logger = std::make_shared<spdlog::async_logger>(
        "main", sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block  // park caller on full queue; never drop
    );

    logger->set_level(spdlog::level::trace);

    // Flush immediately on error or critical.
    logger->flush_on(spdlog::level::trace);

    // Periodic flush: uncomment to drain low-severity messages that never
    // reach the flush_on threshold (useful for long-lived renderer sessions).
    // spdlog::flush_every(ASYNC_FLUSH_INTERVAL);

    // Register before set_default_logger (see setup_logger for the reasoning).
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);

    // Drain the queue and join the worker before static destructors run.
    // drop_all() releases all shared_ptrs from the registry so the thread
    // pool is idle (no in-flight records) when shutdown() joins the worker.
    std::atexit([]() noexcept {
        spdlog::drop_all();  // release logger refs; empties the async queue
        spdlog::shutdown();  // join the worker thread
    });
}

/**
 * @brief Initialises the logging system with the synchronous backend.
 *
 * @details Installs my_error_handler, calls setup_logger(), and catches every
 *          exception that can emerge from spdlog internals.  On failure an
 *          error is written to std::cerr and execution continues.
 *
 * @pre Call once at program startup before any LTRACE/LINFO/… macro.
 * @post The default spdlog logger is configured with a coloured stdout sink.
 *
 * @see INIT_LOG_ASYNC, setup_logger
 *
 * @par Example:
 * @code{.cpp}
 * int main() {
 *     INIT_LOG();
 *     LINFO("Application starting (sync logging).");
 * }
 * @endcode
 */
#define INIT_LOG()                                                                                                                         \
    do {                                                                                                                                   \
        spdlog::set_error_handler(my_error_handler);                                                                                       \
        try {                                                                                                                              \
            setup_logger();                                                                                                                \
        } catch(const spdlog::spdlog_ex &ex) {                                                                                             \
            std::cerr << "Logger initialization failed: " << ex.what() << '\n';                                                            \
        } catch(const std::exception &e) { std::cerr << "Unhandled exception: " << e.what() << '\n'; } catch(...) {                        \
            std::cerr << "An unknown error occurred. Logger initialization failed.\n";                                                     \
        }                                                                                                                                  \
    } while(0)
/**
 * @brief Initialises the logging system with the asynchronous backend.
 *
 * @details Installs my_error_handler before setup_logger_async() so that
 *          thread-pool errors are routed to the handler immediately.  Catches
 *          every exception that can emerge from spdlog internals.
 *
 *          Queue draining and worker-thread shutdown are handled automatically
 *          by the std::atexit handler registered inside setup_logger_async().
 *          Call sites do not need any explicit cleanup.
 *
 * @pre Call once at program startup before any LTRACE/LINFO/… macro.
 * @pre Do not mix INIT_LOG and INIT_LOG_ASYNC in the same process.
 * @post The default spdlog logger is an async_logger writing on a background
 *       thread; the atexit handler will drain and shut it down on exit.
 *
 * @see INIT_LOG, setup_logger_async
 *
 * @par Example:
 * @code{.cpp}
 * int main() {
 *     INIT_LOG_ASYNC();
 *     LINFO("Application starting (async logging).");
 *     // No explicit shutdown needed; atexit drains and joins the worker.
 * }
 * @endcode
 */
#define INIT_LOG_ASYNC()                                                                                                                   \
    do {                                                                                                                                   \
        spdlog::set_error_handler(my_error_handler);                                                                                       \
        try {                                                                                                                              \
            setup_logger_async();                                                                                                                \
        } catch(const spdlog::spdlog_ex &ex) {                                                                                             \
            std::cerr << "Logger initialization failed: " << ex.what() << '\n';                                                            \
        } catch(const std::exception &e) { std::cerr << "Unhandled exception: " << e.what() << '\n'; } catch(...) {                        \
            std::cerr << "An unknown error occurred. Logger initialization failed.\n";                                                     \
        }                                                                                                                                  \
    } while(0)

/// @}
// NOLINTEND(*-include-cleaner)
