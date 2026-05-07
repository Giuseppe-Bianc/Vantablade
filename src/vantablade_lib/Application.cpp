/*
 * Created by gbian on 02/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix)
#include "Vantablade/Application.hpp"
#include "Vantablade/FPSCounter.hpp"
#include "Vantablade/vulkanCheck.hpp"

Application::Application() {
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

Application::~Application() { 
    const vnd::AutoTimer timer{"Destroying Application"};
    vkDestroyPipelineLayout(device_m.device(), pipelineLayout, nullptr); 
}

void Application::run() {
    //initWindow();
    //initVulkan();
    mainLoop();
    //cleanup();
}

void Application::mainLoop() {
    FPSCounter fps{window.getGLFWWindow(), wtile};
    while(!window.shouldClose()) [[likely]] {
        glfwPollEvents();
        drawFrame();
        fps.frameInTitle(false, true);
    }
    VK_CHECK(vkDeviceWaitIdle(device_m.device()), "failed to wait for device idle!");
}

void Application::createPipelineLayout() {
    const vnd::AutoTimer timer{"Creating pipeline layout"};

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                        .setLayoutCount = 0,
                                                        .pSetLayouts = nullptr,
                                                        .pushConstantRangeCount = 0,
                                                        .pPushConstantRanges = nullptr};

    // STYLE: VK_CHECK replaces manual if-check for consistency with the rest
    // of the codebase.
    VK_CHECK(vkCreatePipelineLayout(device_m.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout), "failed to create pipeline layout!");
}

void Application::createPipeline() {
    const vnd::AutoTimer timer{"Creating pipeline"};

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig, swapChain.width(), swapChain.height());
    pipelineConfig.renderPass = swapChain.getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;

    pipeline = std::make_unique<Pipeline>(device_m, "shaders/simple_shader.vert.opt.spv", "shaders/simple_shader.frag.opt.spv", pipelineConfig);
}

void Application::createCommandBuffers() {
    const vnd::AutoTimer timer{"Creating command buffers"};
    commandBuffers.resize(swapChain.imageCount());

    const VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                .commandPool = device_m.getCommandPool(),
                                                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                .commandBufferCount = C_UI32T(commandBuffers.size())};

    VK_CHECK(vkAllocateCommandBuffers(device_m.device(), &allocInfo, commandBuffers.data()), "failed to allocate command buffers!");

    for(const auto &[i, cmdBuf] : std::views::enumerate(commandBuffers)) {
        const VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

        VK_CHECK(vkBeginCommandBuffer(cmdBuf, &beginInfo), "failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain.getRenderPass();
        renderPassInfo.framebuffer = swapChain.getFrameBuffer(C_ST(i));
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain.getSwapChainExtent();

        const std::array<VkClearValue, 2> clearValues{VkClearValue{.color = {.float32 = {0.1f, 0.1f, 0.1f, 1.0f}}},
                                                      VkClearValue{.depthStencil = {.depth = 1.0f, .stencil = 0}}};

        renderPassInfo.clearValueCount = C_UI32T(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        pipeline->bind(cmdBuf);
        vkCmdDraw(cmdBuf, 3, 1, 0, 0);

        vkCmdEndRenderPass(cmdBuf);
        VK_CHECK(vkEndCommandBuffer(cmdBuf), "failed to record command buffer!");
    }
}

void Application::drawFrame() {
    uint32_t imageIndex = 0;

    // CONST: acquireResult is not reassigned after this call.
    const VkResult acquireResult = swapChain.acquireNextImage(&imageIndex);
    VK_CHECK_SWAPCHAIN(acquireResult, "failed to acquire swap chain image!");

    // CONST: submitResult is not reassigned after this call.
    const VkResult submitResult = swapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    VK_CHECK(submitResult, "failed to present swap chain image!");
}

// NOLINTEND(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix)