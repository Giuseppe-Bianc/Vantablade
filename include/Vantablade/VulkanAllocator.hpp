/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

#include "Vantablade/headers.hpp"

namespace vnd {
    static constexpr std::size_t scopeCount = 5;  // COMMAND, OBJECT, CACHE, DEVICE, INSTANCE

    /**
     * @class VulkanAllocator
     * @brief CPU-side memory tracker for Vulkan allocations.
     *
     * Implements the logic described in Kyle Halladay's "Custom Allocators in Vulkan"
     * to track and log CPU memory usage by the Vulkan driver.
     */
    class VulkanAllocator final {
    public:
        VulkanAllocator() = default;
        ~VulkanAllocator() = default;

        // Not copyable or movable
        VulkanAllocator(const VulkanAllocator &) = delete;
        VulkanAllocator &operator=(const VulkanAllocator &) = delete;
        VulkanAllocator(VulkanAllocator &&) = delete;
        VulkanAllocator &operator=(VulkanAllocator &&) = delete;

        [[nodiscard]] VkAllocationCallbacks getCallbacks() noexcept {
            return VkAllocationCallbacks{.pUserData = this,
                                         .pfnAllocation = &vklAllocation,
                                         .pfnReallocation = &vklReallocation,
                                         .pfnFree = &vklFree,
                                         .pfnInternalAllocation = nullptr,
                                         .pfnInternalFree = nullptr};
        }

        struct ScopeSnapshot final {
            std::size_t liveBytes{};
            std::size_t peakBytes{};
            std::size_t allocCount{};
            std::size_t freeCount{};
            std::size_t reallocCount{};
            std::size_t failCount{};
        };

        [[nodiscard]] std::size_t getTotalAllocated() const noexcept { return totalAllocated.load(std::memory_order_relaxed); }

        [[nodiscard]] ScopeSnapshot getScopeSnapshot(VkSystemAllocationScope scope) const noexcept;

        void dumpReport() const;

    private:
        [[nodiscard]] static constexpr std::string_view scopeName(VkSystemAllocationScope scope) noexcept;
        void dumpOneScope(VkSystemAllocationScope scope) const;
        static VKAPI_ATTR void *VKAPI_CALL vklAllocation(void *pUserData, std::size_t size, std::size_t alignment,
                                                         VkSystemAllocationScope allocationScope) noexcept;

        static VKAPI_ATTR void *VKAPI_CALL vklReallocation(void *pUserData, void *pOriginal, std::size_t size, std::size_t alignment,
                                                           VkSystemAllocationScope allocationScope) noexcept;

        static VKAPI_ATTR void VKAPI_CALL vklFree(void *pUserData, void *pMemory) noexcept;

        struct ScopeStats final {
            std::atomic<std::size_t> liveBytes{0};
            std::atomic<std::size_t> peakBytes{0};
            std::atomic<std::size_t> allocCount{0};
            std::atomic<std::size_t> freeCount{0};
            std::atomic<std::size_t> reallocCount{0};
            std::atomic<std::size_t> failCount{0};
        };

        [[nodiscard]] static constexpr std::size_t scopeIndex(VkSystemAllocationScope scope) noexcept {
            const auto v = static_cast<std::size_t>(std::to_underlying(scope));
            return (v < scopeCount) ? v : 0;
        }

        static void updatePeak(std::atomic<std::size_t> &peak, std::size_t candidate) noexcept;

        std::atomic<std::size_t> totalAllocated{0};
        std::array<ScopeStats, scopeCount> scopes{};
    };

}  // namespace vnd