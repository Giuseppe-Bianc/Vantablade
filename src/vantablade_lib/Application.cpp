/*
 * Created by gbian on 02/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers,
// *-magic-numbers)
#include "Vantablade/Application.hpp"
#include "Vantablade/FPSCounter.hpp"
#include "Vantablade/vulkanCheck.hpp"

DISABLE_WARNINGS_PUSH(4324)
struct SimplePushConstantData {
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

DISABLE_WARNINGS_POP()

Application::Application() {
    loadModels();
    createPipelineLayout();
    recreateSwapChain();
    createCommandBuffers();
}

Application::~Application() {
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Destroying Application"};
#endif
    vkDestroyPipelineLayout(device_m.device(), pipelineLayout, nullptr);
}

void Application::run() {
    // initWindow();
    // initVulkan();
    mainLoop();
    // cleanup();
}

void Application::loadModels() {
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Loading models"};
#endif
    // clang-format off
    const std::vector<Model::Vertex> vertices{
        {.position={0.0f, -0.5f}, .color={1.0f, 0.0f, 0.0f}},
        {.position={0.5f, 0.5f}, .color={0.0f, 1.0f, 0.0f}},
        {.position={-0.5f, 0.5f}, .color={0.0f, 0.0f, 1.0f}}};
    // clang-format on
    model = std::make_unique<Model>(device_m, vertices);
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
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Creating pipeline layout"};
#endif

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                        .pNext = nullptr,
                                                        .flags = 0,
                                                        .setLayoutCount = 0,
                                                        .pSetLayouts = nullptr,
                                                        .pushConstantRangeCount = 1,
                                                        .pPushConstantRanges = &pushConstantRange};
    VK_CHECK(vkCreatePipelineLayout(device_m.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout), "failed to create pipeline layout!");
    device_m.setObjectName(pipelineLayout, "Main PipelineLayout");
}

void Application::recreateSwapChain() {
    auto extent = window.getExtent();
    while(extent.width == 0 || extent.height == 0) {
        extent = window.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device_m.device());

    if(swapChain == nullptr) {
        swapChain = std::make_unique<SwapChain>(device_m, extent);
    } else {
        swapChain = std::make_unique<SwapChain>(device_m, extent, std::move(swapChain));
        if(swapChain->imageCount() != commandBuffers.size()) {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }

    createPipeline();
}

void Application::createPipeline() {
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Creating pipeline"};
#endif
    assert(swapChain != nullptr && "Cannot create pipeline before swap chain");
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = swapChain->getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;

    pipeline = std::make_unique<Pipeline>(
        device_m, calculateRelativePathToShaders(Vantablade::cmake::project_path(), "simple_shader.vert.opt.spv"),
        calculateRelativePathToShaders(Vantablade::cmake::project_path(), "simple_shader.frag.opt.spv"), pipelineConfig);
}

void Application::createCommandBuffers() {
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Creating command buffers"};
#endif
    commandBuffers.resize(swapChain->imageCount());

    const VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                .pNext = nullptr,
                                                .commandPool = device_m.getCommandPool(),
                                                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                .commandBufferCount = C_UI32T(commandBuffers.size())};

    VK_CHECK(vkAllocateCommandBuffers(device_m.device(), &allocInfo, commandBuffers.data()), "failed to allocate command buffers!");

    for(const auto &[i, commandBuffer] : std::views::enumerate(commandBuffers)) {
        const auto name = FORMAT("Main CommandBuffer[{}]", i);
        device_m.setObjectName(commandBuffer, name.c_str());
    }
}

void Application::freeCommandBuffers() {
    vkFreeCommandBuffers(device_m.device(), device_m.getCommandPool(), C_UI32T(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}

void Application::recordCommandBuffer(int imageIndex) {
    static int frame = 30;
    frame = (frame + 1) % 100;

    const VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = 0, .pInheritanceInfo = nullptr};

    VK_CHECK(vkBeginCommandBuffer(commandBuffers[C_ST(imageIndex)], &beginInfo), "failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(imageIndex);

    renderPassInfo.renderArea.offset = {.x = 0, .y = 0};
    renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = {VkClearValue{.color = VkClearColorValue{{0.01f, 0.01f, 0.01f, 1.0f}}},
                                               VkClearValue{.depthStencil = VkClearDepthStencilValue{1.0f, 0}}};
    renderPassInfo.clearValueCount = C_UI32T(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[C_ST(imageIndex)], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = C_F(swapChain->getSwapChainExtent().width);
    viewport.height = C_F(swapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    const VkRect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffers[C_ST(imageIndex)], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers[C_ST(imageIndex)], 0, 1, &scissor);

    pipeline->bind(commandBuffers[C_ST(imageIndex)]);
    model->bind(commandBuffers[C_ST(imageIndex)]);

    for(int j = 0; j < 4; j++) {
        SimplePushConstantData push{};
        push.offset = {-0.5f + C_F(frame) * 0.02f, -0.4f + C_F(j) * 0.25f};
        push.color = {0.0f, 0.0f, 0.2f + 0.2f * C_F(j)};

        vkCmdPushConstants(commandBuffers[C_ST(imageIndex)], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(SimplePushConstantData), &push);
        model->draw(commandBuffers[C_ST(imageIndex)]);
    }

    vkCmdEndRenderPass(commandBuffers[C_ST(imageIndex)]);
    VK_CHECK(vkEndCommandBuffer(commandBuffers[C_ST(imageIndex)]), "failed to record command buffer!");
}

void Application::drawFrame() {
    uint32_t imageIndex = 0;
    auto acquireResult = swapChain->acquireNextImage(&imageIndex);

    if(acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }

    VK_CHECK_SWAPCHAIN(acquireResult, "failed to acquire swap chain image!");

    recordCommandBuffer(NC_I(imageIndex));
    auto submitResult = swapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if(submitResult == VK_ERROR_OUT_OF_DATE_KHR || submitResult == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
        window.resetWindowResizedFlag();
        recreateSwapChain();
        return;
    } else if(submitResult != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

// NOLINTEND(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers,
// *-magic-numbers)
