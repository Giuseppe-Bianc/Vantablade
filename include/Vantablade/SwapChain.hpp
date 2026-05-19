/*
 * Created by gbian on 07/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "Device.hpp"

class SwapChain {
public:
    static inline constexpr std::size_t MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(Device &deviceRef, VkExtent2D extent);
    SwapChain(Device &deviceRef, VkExtent2D extent, std::shared_ptr<SwapChain> previous);
    ~SwapChain();

    SwapChain(const SwapChain &) = delete;
    SwapChain &operator=(const SwapChain &) = delete;

    [[nodiscard]] VkFramebuffer getFrameBuffer(std::size_t index) const { return swapChainFramebuffers.at(index); }
    [[nodiscard]] VkRenderPass getRenderPass() const noexcept { return renderPass; }
    [[nodiscard]] VkImageView getImageView(std::size_t index) const { return swapChainImageViews.at(index); }
    [[nodiscard]] std::size_t imageCount() const noexcept { return swapChainImages.size(); }
    [[nodiscard]] VkFormat getSwapChainImageFormat() const noexcept { return swapChainImageFormat; }
    [[nodiscard]] VkExtent2D getSwapChainExtent() const noexcept { return swapChainExtent; }
    [[nodiscard]] uint32_t width() const noexcept { return swapChainExtent.width; }
    [[nodiscard]] uint32_t height() const noexcept { return swapChainExtent.height; }

    // CONST + [[nodiscard]]: pure arithmetic query — was neither const nor nodiscard.
    [[nodiscard]] float extentAspectRatio() const noexcept { return C_F(swapChainExtent.width) / C_F(swapChainExtent.height); }

    [[nodiscard]] VkFormat findDepthFormat() const;

    [[nodiscard]] VkResult acquireNextImage(uint32_t *imageIndex);
    [[nodiscard]] VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // CONST: helper functions do not modify *this.
    [[nodiscard]] VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const;
    [[nodiscard]] static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    [[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    VkFormat swapChainImageFormat{VK_FORMAT_UNDEFINED};
    VkExtent2D swapChainExtent{0, 0};

    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass renderPass{VK_NULL_HANDLE};

    std::vector<VkImage> depthImages;
    std::vector<VmaAllocation> depthImageAllocations;
    std::vector<VkImageView> depthImageViews;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    Device &device;
    VkExtent2D windowExtent;

    VkSwapchainKHR swapChain{VK_NULL_HANDLE};
    std::shared_ptr<SwapChain> oldSwapChain;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    std::size_t currentFrame{0};
};