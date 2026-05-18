/*
 * Created by gbian on 06/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers,*-magic-numbers)
#include "Vantablade/FPSCounter.hpp"

FPSCounter::FPSCounter(GLFWwindow *window, std::string_view title) : last_time(vnd::clock::now()), m_window(window), m_title(title) {
    ms_per_frameComposition.reserve(32);
    titleBuffer.reserve(128);
}

void FPSCounter::frame(const bool vsync, const bool showMax) {
    updateFPS();
    ensureMsPerFrameStringUpToDate();

    const auto *const vsyncText = vsync ? "Enabled" : "Disabled";
    if(showMax) {
        LINFO("{:.3LF} fps/{} - Max: {:.3LF} - VSync: {}", fps, ms_per_frameComposition, max_fps, vsyncText);
    } else {
        LINFO("{:.3LF} fps/{} - VSync: {}", fps, ms_per_frameComposition, vsyncText);
    }
}

void FPSCounter::frameInTitle(const bool vsync, const bool showMax) {
    updateFPS();
    ensureMsPerFrameStringUpToDate();

    // If caller passed a null window, the old code would still call GLFW and likely crash.
    // Keeping behavior means no added guard here.

    const auto *const vsyncText = vsync ? "Enabled" : "Disabled";

    // PERF: reuse titleBuffer to avoid creating a temporary string each call.
    titleBuffer.clear();

    if(showMax) {
        titleBuffer = FORMAT("{} - {:.3LF} fps/{} - Max: {:.3LF} - VSync: {}", m_title, fps, ms_per_frameComposition, max_fps, vsyncText);
    } else {
        titleBuffer = FORMAT("{} - {:.3LF} fps/{} - VSync: {}", m_title, fps, ms_per_frameComposition, vsyncText);
    }

    glfwSetWindowTitle(m_window, titleBuffer.c_str());
}

void FPSCounter::updateFPS() noexcept {
    const auto current_time = vnd::clock::now();

    ++frames;
    const auto time_step = vnd::TimeValues(ch::duration_cast<vnd::nanolld>(current_time - last_time).count());

    last_time = current_time;

    frameTime = time_step.get_seconds();
    totalTime += frameTime;

    if(totalTime >= 1.0L) {
        const auto ldframes = C_LD(frames);

        fps = ldframes / totalTime;
        ms_per_frame = totalTime / ldframes;

        frames = 0;
        totalTime = 0.0L;

        max_fps = std::max(max_fps, fps);
    }

    // PERF: no formatting here. Presentation is lazy.
}

void FPSCounter::ensureMsPerFrameStringUpToDate() {
    if(ms_per_frame == ms_per_frame_last_formatted) { return; }
    ms_per_frame_last_formatted = ms_per_frame;
    formatTimeInto(ms_per_frameComposition, ms_per_frame);
}

DISABLE_WARNINGS_PUSH(26447)
void FPSCounter::formatTimeInto(std::string &out, const long double inputTimeMilli) {
    using namespace std::chrono;

    const auto durationmils = duration<long double, std::milli>(inputTimeMilli);

    const auto durationMs = floor<milliseconds>(durationmils);
    const auto remainderAfterMs = durationmils - durationMs;

    const auto durationUs = floor<microseconds>(remainderAfterMs);
    const auto remainderAfterUs = remainderAfterMs - durationUs;

    const auto durationNs = round<nanoseconds>(remainderAfterUs);

    // PERF: reuse out buffer. FORMAT likely allocates, so we assign into an existing string.
    out = FORMAT("{}ms,{}us,{}ns", C_LD(durationMs.count()), C_LD(durationUs.count()), C_LD(durationNs.count()));
}
DISABLE_WARNINGS_POP()

long double FPSCounter::getFPS() const noexcept { return fps; }

long double FPSCounter::getMsPerFrame() const noexcept { return ms_per_frame; }

// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers,*-magic-numbers)