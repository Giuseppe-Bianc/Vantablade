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

Application::~Application() { vkDestroyPipelineLayout(device_m.device(), pipelineLayout, nullptr); }
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
    vkDeviceWaitIdle(device_m.device());
}

void Application::createPipelineLayout() {
    const vnd::AutoTimer timer{"Creating pipeline layout"};
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;
  if (vkCreatePipelineLayout(device_m.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void Application::createPipeline() {
    const vnd::AutoTimer timer{"Creating pipeline"};
  PipelineConfigInfo pipelineConfig{};
  Pipeline::defaultPipelineConfigInfo(
      pipelineConfig,
      swapChain.width(),
      swapChain.height());
  pipelineConfig.renderPass = swapChain.getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  pipeline = std::make_unique<Pipeline>(
      device_m,
      "shaders/simple_shader.vert.opt.spv", "shaders/simple_shader.frag.opt.spv",
      pipelineConfig);
}

void Application::createCommandBuffers() {
    const vnd::AutoTimer timer{"Creating command buffers"};
  commandBuffers.resize(swapChain.imageCount());

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = device_m.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(device_m.device(), &allocInfo, commandBuffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }

  for (int i = 0; i < commandBuffers.size(); i++) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo), "failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain.getRenderPass();
    renderPassInfo.framebuffer = swapChain.getFrameBuffer(i);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain.getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    pipeline->bind(commandBuffers[i]);
    vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[i]);
    VK_CHECK(vkEndCommandBuffer(commandBuffers[i]), "failed to record command buffer!");
  }
}
void Application::drawFrame() {
  uint32_t imageIndex;
  auto result = swapChain.acquireNextImage(&imageIndex);
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  result = swapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
  VK_CHECK(result, "failed to present swap chain image!");
}

// NOLINTEND(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix)