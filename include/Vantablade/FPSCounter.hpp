/*
* Created by gbian on 06/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

#pragma once

#include "headers.hpp"

class FPSCounter {
public:
    explicit FPSCounter(GLFWwindow* window, std::string_view title = "title");

    void frame(bool vsync = false, bool showMax = true);
    void frameInTitle(bool vsync = false, bool showMax = true);

    void updateFPS() noexcept;

    [[nodiscard]] long double getFPS() const noexcept;
    [[nodiscard]] long double getFrameTime() const noexcept { return frameTime; }
    [[nodiscard]] long double getMsPerFrame() const noexcept;

private:
    // PERF: only regenerate the time string when ms_per_frame changes (about once per second).
    void ensureMsPerFrameStringUpToDate();

    // PERF: write into an existing string buffer to avoid per-call allocations.
    static void formatTimeInto(std::string& out, long double inputTimeMilli);

    vnd::time_point last_time{};
    int frames = 0;
    long double fps = 0.0L;
    long double max_fps = 0.0L;
    long double ms_per_frame = 0.0L;
    long double frameTime = 0.0L;
    long double totalTime = 0.0L;

    GLFWwindow* m_window = nullptr;          // non-owning
    std::string_view m_title{};              // non-owning, caller must guarantee lifetime

    std::string ms_per_frameComposition{};
    long double ms_per_frame_last_formatted = -1.0L;

    // PERF: reuse a buffer for the title to avoid per-frame allocations in frameInTitle.
    std::string titleBuffer{};
};
