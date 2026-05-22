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
// clang-format on

class Application {
public:
    Application();
    ~Application();

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
};
