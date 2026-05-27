/*
 * Created by gbian on 02/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers,
// *-magic-numbers)
#include "Vantablade/Application.hpp"
#include "Vantablade/FPSCounter.hpp"
#include "Vantablade/Camera.hpp"
#include "Vantablade/SimpleRenderSystem.hpp"
#include "Vantablade/vulkanCheck.hpp"

#include <imgui.h>

class RendererPanel : public IUIPanel {
public:
    RendererPanel(Device &device, const std::vector<GameObject> &gameObjects) : device_m{device}, gameObjects_m{gameObjects} {}

    void onDraw() override {
        ImGui::Begin("Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if(ImGui::CollapsingHeader("Device")) {
            const auto &p = device_m.properties;

            const auto apiVer = FORMAT("{}.{}.{}", VK_API_VERSION_MAJOR(p.apiVersion), VK_API_VERSION_MINOR(p.apiVersion),
                                       VK_API_VERSION_PATCH(p.apiVersion));

            const auto driverVer = FORMAT("{}.{}.{}", VK_API_VERSION_MAJOR(p.driverVersion), VK_API_VERSION_MINOR(p.driverVersion),
                                          VK_API_VERSION_PATCH(p.driverVersion));

            const auto vendor = FORMAT("{}(ID:{:#010x})", getVendorName(p.vendorID), p.vendorID);
            const auto deviceId = FORMAT("{}({:#010x})", p.deviceID, p.deviceID);
            const auto uuid = uuid_to_string(std::span<const uint8_t, 16>{p.pipelineCacheUUID, 16});

            ImGui::Text("Device Name: %s, DeviceID: %s", p.deviceName, deviceId.c_str());
            ImGui::Text("API Version:   %s, Driver Version: %s", apiVer.c_str(), driverVer.c_str());
            ImGui::Text("Vendor:        %s", vendor.c_str());
            ImGui::Text("Device Type:   %s", getDeviceType(p.deviceType));
            ImGui::Text("Pipeline UUID: %s", uuid.c_str());
        }

        ImGui::Separator();
        ImGui::Text("Rendered Objects: %zu", gameObjects_m.size());

        if(!gameObjects_m.empty()) {
            /*for (size_t i = 0; i < gameObjects_m.size(); ++i) {
                ImGui::Text("Object %zu", i);
            }*/
            for(const auto &obj : gameObjects_m) {
                ImGui::Text("Object %u vertices: %u", obj.getId(), obj.model ? obj.model->getVertexCount() : 0);
            }
        }

        ImGui::End();
    }

    const std::string &getName() const override {
        static const std::string name = "Renderer";
        return name;
    }

private:
    Device &device_m;
    const std::vector<GameObject> &gameObjects_m;
};

class FPSPanel : public IUIPanel {
public:
    explicit FPSPanel(FPSCounter &fps) : fps_m{fps} {}

    void onDraw() override {
        ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        const auto fpsLine = FORMAT("{:.3LF} fps/{}", fps_m.getFPS(), fps_m.getMsPerFrameString());
        const auto maxLine = FORMAT("Max: {:.3LF} fps", fps_m.getMaxFPS());
        ImGui::Text("%s", fpsLine.c_str());
        ImGui::Text("%s", maxLine.c_str());

        ImGui::End();
    }

    const std::string &getName() const override {
        static const std::string name = "Performance";
        return name;
    }

private:
    FPSCounter &fps_m;
};

Application::Application() {
    imguiLayer_m = std::make_unique<ImGuiLayer>(device_m, window, renderer_m.getSwapChainRenderPass(), renderer_m.getSwapChainImageCount());
    imguiLayer_m->onAttach();
    imguiLayer_m->addPanel<RendererPanel>(device_m, gameObjects);
    imguiLayer_m->addPanel<FPSPanel>(fps_m);
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
    cube.transform.translation = {.0f, .0f, 2.5f};
    cube.transform.scale = {.5f, .5f, .5f};
    gameObjects.push_back(std::move(cube));
}

void Application::mainLoop() {
    SimpleRenderSystem simpleRenderSystem{device_m, renderer_m.getSwapChainRenderPass()};
    Camera camera{};

    while(!window.shouldClose()) [[likely]] {
        glfwPollEvents();

        float aspect = renderer_m.getAspectRatio();
        // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

        // --- ImGui new frame --------------------------------------------
        imguiLayer_m->begin();

        // --- Vulkan render ----------------------------------------------
        if(auto commandBuffer = renderer_m.beginFrame()) {
            renderer_m.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);

            // ImGui draw calls recorded into the same command buffer,
            // inside the active render pass.
            imguiLayer_m->end(commandBuffer);

            renderer_m.endSwapChainRenderPass(commandBuffer);
            renderer_m.endFrame();
        }

        fps_m.tick();
    }

    VK_CHECK(vkDeviceWaitIdle(device_m.device()), "failed to wait for device idle!");
}

// NOLINTEND(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers,
// *-magic-numbers)
