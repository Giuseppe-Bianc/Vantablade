/*
 * Created by gbian on 02/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers,
// *-magic-numbers)
#include "Vantablade/Application.hpp"
#include "Vantablade/FPSCounter.hpp"
#include "Vantablade/SimpleRenderSystem.hpp"
#include "Vantablade/vulkanCheck.hpp"

#include <imgui.h>

Application::Application() {
    imguiLayer_m = std::make_unique<ImGuiLayer>(device_m, window, renderer_m.getSwapChainRenderPass(), renderer_m.getSwapChainImageCount());
    imguiLayer_m->onAttach();
    loadGameObjects();
}

void Application::run() {
    // initWindow();
    // initVulkan();
    mainLoop();
    // cleanup();
}

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<Model> createCubeModel(Device &device, glm::vec3 offset) {
    std::vector<Model::Vertex> vertices{

        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

    };
    for(auto &v : vertices) { v.position += offset; }
    return std::make_unique<Model>(device, vertices);
}

void Application::loadGameObjects() {
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Loading game objects"};
#endif
    std::shared_ptr<Model> model = createCubeModel(device_m, {.0f, .0f, .0f});
    auto cube = GameObject::createGameObject();
    cube.model = model;
    cube.transform.translation = {.0f, .0f, .5f};
    cube.transform.scale = {.5f, .5f, .5f};
    gameObjects.push_back(std::move(cube));
}

void Application::mainLoop() {
    FPSCounter fps{window.getGLFWWindow(), wtile};
    SimpleRenderSystem simpleRenderSystem{device_m, renderer_m.getSwapChainRenderPass()};

    while(!window.shouldClose()) [[likely]] {
        glfwPollEvents();

        // --- ImGui new frame --------------------------------------------
        imguiLayer_m->begin();

        // --- Renderer stats panel ---------------------------------------
        {
            ImGui::Begin("Renderer");

            const auto fpsLine = FORMAT("{:.3LF} fps/{}", fps.getFPS(), fps.getMsPerFrameString());
            const auto maxLine = FORMAT("Max: {:.3LF} fps", fps.getMaxFPS());
            ImGui::Text("%s", fpsLine.c_str());
            ImGui::Text("%s", maxLine.c_str());

            ImGui::Separator();
            ImGui::Text("Device: %s", device_m.properties.deviceName);
            ImGui::Text("Objects: %zu", gameObjects.size());

            ImGui::End();
        }

        // Add further panels here — each in its own ImGui::Begin/End block.

        // --- Vulkan render ----------------------------------------------
        if(auto commandBuffer = renderer_m.beginFrame()) {
            renderer_m.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);

            // ImGui draw calls recorded into the same command buffer,
            // inside the active render pass.
            imguiLayer_m->end(commandBuffer);

            renderer_m.endSwapChainRenderPass(commandBuffer);
            renderer_m.endFrame();
        }

        fps.tick();
    }

    VK_CHECK(vkDeviceWaitIdle(device_m.device()), "failed to wait for device idle!");
}

// NOLINTEND(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers,
// *-magic-numbers)
