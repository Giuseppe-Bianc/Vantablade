/*
 * Created by gbian on 22/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers,*-magic-numbers, *-uppercase-literal-suffix, *-pro-type-union-access)
#include "Vantablade/Renderer.hpp"

#include "Vantablade/vulkanCheck.hpp"

Renderer::Renderer(Window &window, Device &device) : window_m(window), device_m(device) {
    VZ_ZONE_SCOPED;
    recreateSwapChain();
    createCommandBuffers();
}
Renderer::~Renderer() {
    VZ_ZONE_SCOPED;
    freeCommandBuffers();
}

uint32_t Renderer::getSwapChainImageCount() const {
    VZ_ZONE_SCOPED;
    return C_UI32T(swapChain_m->imageCount());
}

void Renderer::recreateSwapChain() {
    VZ_ZONE_SCOPED;
    auto extent = window_m.getExtent();
    while(extent.width == 0 || extent.height == 0) {
        extent = window_m.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device_m.device());

    if(swapChain_m == nullptr) {
        swapChain_m = std::make_unique<SwapChain>(device_m, extent);
    } else {
        const std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain_m);
        swapChain_m = std::make_unique<SwapChain>(device_m, extent, oldSwapChain);

        if(!oldSwapChain->compareSwapFormats(*swapChain_m)) { throw std::runtime_error("Swap chain image(or depth) format has changed!"); }
    }
}

void Renderer::createCommandBuffers() {
    VZ_ZONE_SCOPED;
    commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device_m.getCommandPool();
    allocInfo.commandBufferCount = C_UI32T(commandBuffers.size());

    VK_CHECK(vkAllocateCommandBuffers(device_m.device(), &allocInfo, commandBuffers.data()), "failed to allocate command buffers!");
    for(const auto &[i, commandBuffer] : std::views::enumerate(commandBuffers)) {
        const auto name = FORMAT("Main Render CommandBuffer[{}]", i);
        device_m.setObjectName(commandBuffer, name.c_str());
    }
}

void Renderer::freeCommandBuffers() {
    VZ_ZONE_SCOPED;
    vkFreeCommandBuffers(device_m.device(), device_m.getCommandPool(), C_UI32T(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}

VkCommandBuffer Renderer::beginFrame() {
    VZ_ZONE_SCOPED;
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = swapChain_m->acquireNextImage(&currentImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }

    VK_CHECK_SWAPCHAIN(result, "failed to acquire swap chain image!");

    isFrameStarted = true;

    auto *commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo), "failed to begin recording command buffer!");
    device_m.getProfiler().beginGpuZone(commandBuffer, "MainFrame");
    return commandBuffer;
}

void Renderer::endFrame() {
    VZ_ZONE_SCOPED;
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto *commandBuffer = getCurrentCommandBuffer();
    device_m.getProfiler().endGpuZone(commandBuffer);
    VK_CHECK(vkEndCommandBuffer(commandBuffer), "failed to record command buffer!");

    auto result = swapChain_m->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_m.wasWindowResized()) {
        window_m.resetWindowResizedFlag();
        recreateSwapChain();
    } else if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % C_I(SwapChain::MAX_FRAMES_IN_FLIGHT);

    device_m.getProfiler().resolveTimestamps();
    device_m.updateMemoryStats();
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    VZ_ZONE_SCOPED;
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain_m->getRenderPass();
    renderPassInfo.framebuffer = swapChain_m->getFrameBuffer(currentImageIndex);

    renderPassInfo.renderArea.offset = {.x = 0, .y = 0};
    renderPassInfo.renderArea.extent = swapChain_m->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
    clearValues[1].depthStencil = {.depth = 1.0f, .stencil = 0};
    renderPassInfo.clearValueCount = C_UI32T(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = C_F(swapChain_m->getSwapChainExtent().width);
    viewport.height = C_F(swapChain_m->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    const VkRect2D scissor{{0, 0}, swapChain_m->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

// NOLINTNEXTLINE(*-convert-member-functions-to-static)
void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) const {
    VZ_ZONE_SCOPED;
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
}

// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers,*-magic-numbers, *-uppercase-literal-suffix, *-pro-type-union-access)
