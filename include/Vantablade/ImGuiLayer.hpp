/*
 * ImGuiLayer — wraps the Dear ImGui Vulkan/GLFW backend.
 * Follows the same attach/detach lifecycle The Cherno uses in Hazel.
 * Call begin() before any ImGui:: API calls, end(cmd) to flush draw data.
 */

#pragma once

#include "Device.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"

class ImGuiLayer {
public:
    ImGuiLayer(Device &device, Window &window, VkRenderPass renderPass, uint32_t imageCount);
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer &) = delete;
    ImGuiLayer &operator=(const ImGuiLayer &) = delete;

    // Call once after construction (or when pushed onto a layer stack).
    void onAttach();

    // Call once before destroying (or when popped from a layer stack).
    // Safe to call multiple times — guarded internally.
    void onDetach();

    // Call at the start of every frame, before any ImGui:: API calls.
    void begin();

    // Call at the end of every frame, inside an active render pass.
    void end(VkCommandBuffer commandBuffer);

    // Update min/max image count after a swap chain recreation.
    // Call this from Renderer::recreateSwapChain if image count changes.
    void onSwapChainRecreated(uint32_t newImageCount);

private:
    void createDescriptorPool();
    static void setupStyle();

    Device &device_m;
    Window &window_m;
    VkRenderPass renderPass_m;
    uint32_t imageCount_m;

    VkDescriptorPool descriptorPool_m{VK_NULL_HANDLE};
    bool attached_m{false};
};