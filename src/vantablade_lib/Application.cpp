/*
 * Created by gbian on 02/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers,
// *-magic-numbers)
#include "Vantablade/Application.hpp"
#include "Vantablade/Camera.hpp"
#include "Vantablade/FPSCounter.hpp"
#include "Vantablade/SimpleRenderSystem.hpp"
#include "Vantablade/vulkanCheck.hpp"

#include <imgui.h>

class RendererPanel : public IUIPanel {
public:
    RendererPanel(Device &device, const std::vector<GameObject> &gameObjects) : device_m{device}, gameObjects_m{gameObjects} {
        // PERF: device properties never change after construction — format once here,
        // never inside onDraw(). Five FORMAT calls per frame become zero.
        const auto &p = device_m.properties;
        apiVer_m = FORMAT("{}.{}.{}", VK_API_VERSION_MAJOR(p.apiVersion), VK_API_VERSION_MINOR(p.apiVersion),
                          VK_API_VERSION_PATCH(p.apiVersion));
        driverVer_m = FORMAT("{}.{}.{}", VK_API_VERSION_MAJOR(p.driverVersion), VK_API_VERSION_MINOR(p.driverVersion),
                             VK_API_VERSION_PATCH(p.driverVersion));
        vendor_m = FORMAT("{}(ID:{:#010x})", getVendorName(p.vendorID), p.vendorID);
        deviceId_m = FORMAT("{}({:#010x})", p.deviceID, p.deviceID);
        // uuid_to_string now produces one allocation; see DeviceInfo.hpp refactoring.
        uuid_m = uuid_to_string(std::span<const uint8_t, 16>{p.pipelineCacheUUID, 16});
    }

    void onDraw() override {
        ImGui::Begin("Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if(ImGui::CollapsingHeader("Device")) {
            // PERF: all strings pre-computed at construction — zero FORMAT calls here.
            ImGui::Text("Device Name: %s, DeviceID: %s", device_m.properties.deviceName, deviceId_m.c_str());
            ImGui::Text("API Version:   %s, Driver Version: %s", apiVer_m.c_str(), driverVer_m.c_str());
            ImGui::Text("Vendor:        %s", vendor_m.c_str());
            ImGui::Text("Device Type:   %s", getDeviceType(device_m.properties.deviceType));
            ImGui::Text("Pipeline UUID: %s", uuid_m.c_str());
        }

        ImGui::Separator();
        ImGui::Text("Rendered Objects: %zu", gameObjects_m.size());

        for(const auto &obj : gameObjects_m) {
            ImGui::Text("Object %u vertices: %u", obj.getId(), obj.model ? obj.model->getVertexCount() : 0u);
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
    // PERF: cached, immutable device info strings.
    std::string apiVer_m;
    std::string driverVer_m;
    std::string vendor_m;
    std::string deviceId_m;
    std::string uuid_m;
};

class FPSPanel : public IUIPanel {
public:
    explicit FPSPanel(FPSCounter &fps) : fps_m{fps} {
        fpsLine_m.reserve(64);
        maxLine_m.reserve(32);
    }

    void onDraw() override {
        ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        // PERF: FPS updates at most once per second (when totalTime >= 1.0L in FPSCounter).
        // Cache the formatted strings and only reformat on value change.
        // Two FORMAT calls per frame become zero in the steady state.
        const long double currentFps = fps_m.getFPS();
        if(std::fabsl(currentFps - lastFps_m) > std::numeric_limits<long double>::epsilon()) {
            lastFps_m = currentFps;
            fpsLine_m = FORMAT("{:.3LF} fps/{}", currentFps, fps_m.getMsPerFrameString());
        }
        const long double currentMax = fps_m.getMaxFPS();
        if(std::fabsl(currentMax - lastMax_m) > std::numeric_limits<long double>::epsilon()) {
            lastMax_m = currentMax;
            maxLine_m = FORMAT("Max: {:.3LF} fps", currentMax);
        }

        ImGui::Text("%s", fpsLine_m.c_str());
        ImGui::Text("%s", maxLine_m.c_str());

        ImGui::End();
    }

    const std::string &getName() const override {
        static const std::string name = "Performance";
        return name;
    }

private:
    FPSCounter &fps_m;
    std::string fpsLine_m;
    std::string maxLine_m;
    long double lastFps_m = -1.0L;
    long double lastMax_m = -1.0L;
};

class CameraPanel : public IUIPanel {
public:
    explicit CameraPanel(const Camera &camera) : camera_m{camera} {}

    void onDraw() override {
        ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        const auto &projection = camera_m.getProjection();
        const auto &view = camera_m.getView();

        ImGui::Text("Projection Matrix: %s", glmp::to_string(projection).c_str());

        ImGui::Text("View Matrix: %s", glmp::to_string(view).c_str());

        ImGui::End();
    }

    const std::string &getName() const override {
        static const std::string name = "Camera";
        return name;
    }

private:
    const Camera &camera_m;
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
[[nodiscard]] std::unique_ptr<Model> createCubeModel(Device &device, glm::vec3 offset) {
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
    VZ_ZONE_SCOPED;
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Loading game objects"};
#endif
    auto cube = GameObject::createGameObject();
    cube.model = createCubeModel(device_m, {.0f, .0f, .0f});
    cube.transform.translation = {.0f, .0f, 2.5f};
    cube.transform.scale = {.5f, .5f, .5f};
    gameObjects.push_back(std::move(cube));
}

void Application::mainLoop() {
    VZ_ZONE_SCOPED;
    SimpleRenderSystem simpleRenderSystem{device_m, renderer_m.getSwapChainRenderPass()};
    Camera camera{};

    camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));
    imguiLayer_m->addPanel<CameraPanel>(camera);

    while(!window.shouldClose()) [[likely]] {
        glfwPollEvents();
        [[maybe_unused]] auto frameTime = C_F(fps_m.getFrameTime());

        const float aspect = renderer_m.getAspectRatio();
        // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
        imguiLayer_m->begin();


        // --- Vulkan render ----------------------------------------------
        if(auto commandBuffer = renderer_m.beginFrame()) {
            // GPU frame scope: must end before vkEndCommandBuffer/collect
            {
                VZ_GPU_ZONE(device_m.getProfiler().getContext(), commandBuffer, "Frame::GPU");
                renderer_m.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);

                imguiLayer_m->end(commandBuffer);

                renderer_m.endSwapChainRenderPass(commandBuffer);
            }
            renderer_m.endFrame();
        }

        fps_m.tick();
        VZ_FRAME_MARK();
    }

    VK_CHECK(vkDeviceWaitIdle(device_m.device()), "failed to wait for device idle!");
}

// NOLINTEND(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers,
// *-magic-numbers)
