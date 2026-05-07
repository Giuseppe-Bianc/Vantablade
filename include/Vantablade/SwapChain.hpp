/*
* Created by gbian on 07/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

#pragma once

#include "Device.hpp"

class SwapChain {
 public:
     static inline constexpr std::size_t MAX_FRAMES_IN_FLIGHT = 2;

     SwapChain(Device &deviceRef, VkExtent2D windowExtent);
     ~SwapChain();

     SwapChain(const SwapChain &) = delete;
     SwapChain &operator=(const SwapChain &) = delete;

    [[nodiscard]] VkFramebuffer getFrameBuffer(size_t index) const { return swapChainFramebuffers.at(index); }
    [[nodiscard]] VkRenderPass getRenderPass() const { return renderPass; }
    [[nodiscard]] VkImageView getImageView(size_t index) const { return swapChainImageViews.at(index); }
    [[nodiscard]] size_t imageCount() const { return swapChainImages.size(); }
    [[nodiscard]] VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
    [[nodiscard]] VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
    [[nodiscard]] uint32_t width() const { return swapChainExtent.width; }
    [[nodiscard]] uint32_t height() const { return swapChainExtent.height; }

     float extentAspectRatio() { return C_F(swapChainExtent.width) / C_F(swapChainExtent.height); }
     [[nodiscard]] VkFormat findDepthFormat() const;

     VkResult acquireNextImage(uint32_t *imageIndex);
     VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

 private:
  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat{VK_FORMAT_UNDEFINED};
  VkExtent2D swapChainExtent{0, 0};

  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkRenderPass renderPass{VK_NULL_HANDLE};

  std::vector<VkImage> depthImages;
  std::vector<VkDeviceMemory> depthImageMemorys;
  std::vector<VkImageView> depthImageViews;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  Device &device;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain{VK_NULL_HANDLE};

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
};