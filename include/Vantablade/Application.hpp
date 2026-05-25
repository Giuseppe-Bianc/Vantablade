/*
 * Created by gbian on 02/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "Window.hpp"
#include "Device.hpp"
#include "Renderer.hpp"
#include "Model.hpp"
#include "GameObject.hpp"
#include "ImGuiLayer.hpp"
// clang-format on

class Application {
public:
    Application();
    ~Application() = default;

    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    void run();

private:
    void mainLoop();
    void loadGameObjects();

    Window window{wwidth, wheight, wtile};
    Device device_m{window};
    Renderer renderer_m{window, device_m};
    std::vector<GameObject> gameObjects;
    // ImGuiLayer must be declared after renderer_m so it initialises last
    // and destructs first — render pass must still be valid during shutdown.
    std::unique_ptr<ImGuiLayer> imguiLayer_m;
};
