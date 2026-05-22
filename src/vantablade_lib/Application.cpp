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

Application::Application() { loadGameObjects(); }

void Application::run() {
    // initWindow();
    // initVulkan();
    mainLoop();
    // cleanup();
}

void Application::loadGameObjects() {
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Loading game objects"};
#endif
    // clang-format off
    const std::vector<Model::Vertex> vertices{
        {.position={0.0f, -0.5f}, .color={1.0f, 0.0f, 0.0f}},
        {.position={0.5f, 0.5f}, .color={0.0f, 1.0f, 0.0f}},
        {.position={-0.5f, 0.5f}, .color={0.0f, 0.0f, 1.0f}}};
    // clang-format on
    auto model = std::make_shared<Model>(device_m, vertices);

    auto triangle = GameObject::createGameObject();
    triangle.model = model;
    triangle.color = {.1f, .8f, .1f};
    triangle.transform2d.translation.x = .2f;
    triangle.transform2d.scale = {2.f, .5f};
    triangle.transform2d.rotation = .25f * glm::two_pi<float>();

    gameObjects.push_back(std::move(triangle));
}

void Application::mainLoop() {
    FPSCounter fps{window.getGLFWWindow(), wtile};
    SimpleRenderSystem simpleRenderSystem{device_m, renderer_m.getSwapChainRenderPass()};
    while(!window.shouldClose()) [[likely]] {
        glfwPollEvents();
        if(auto commandBuffer = renderer_m.beginFrame()) {
            renderer_m.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
            renderer_m.endSwapChainRenderPass(commandBuffer);
            renderer_m.endFrame();
        }
        fps.frameInTitle(false, true);
    }
    VK_CHECK(vkDeviceWaitIdle(device_m.device()), "failed to wait for device idle!");
}
// NOLINTEND(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers,
// *-magic-numbers)
