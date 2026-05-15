/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

#include "Vantablade/headers.hpp"

namespace vnd {

    class VulkanAllocator final {
    public:
        static constexpr std::size_t scopeCount = 5;  // COMMAND, OBJECT, CACHE, DEVICE, INSTANCE

        VulkanAllocator() = default;
        ~VulkanAllocator() = default;

        VulkanAllocator(const VulkanAllocator &) = delete;
        VulkanAllocator &operator=(const VulkanAllocator &) = delete;
        VulkanAllocator(VulkanAllocator &&) = delete;
        VulkanAllocator &operator=(VulkanAllocator &&) = delete;

        [[nodiscard]] VkAllocationCallbacks getCallbacks() noexcept {
            return VkAllocationCallbacks{.pUserData = this,
                                         .pfnAllocation = &vklAllocation,
                                         .pfnReallocation = &vklReallocation,
                                         .pfnFree = &vklFree,
                                         .pfnInternalAllocation = vklInternalAllocation,
                                         .pfnInternalFree = vklInternalFree};
        }

        struct ScopeSnapshot final {
            std::size_t liveBytes{};
            std::size_t peakBytes{};
            std::size_t allocCount{};
            std::size_t freeCount{};
            std::size_t reallocCount{};
            std::size_t failCount{};
            std::size_t internalAllocCount{};
            std::size_t internalFreeCount{};
            std::size_t internalLiveBytes{};
            std::size_t internalPeakBytes{};
        };

        // Live bytes globali (somma delle allocazioni vive tracciate dal callback).
        [[nodiscard]] std::size_t getTotalLiveBytes() const noexcept;

        // Bytes cumulativi allocati con successo (diagnostica, utile per leak rate e churn).
        [[nodiscard]] std::size_t getTotalCumulativeAllocatedBytes() const noexcept;

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

        static VKAPI_ATTR void VKAPI_CALL vklInternalAllocation(void *pUserData, std::size_t size, VkInternalAllocationType allocationType,
                                                                VkSystemAllocationScope allocationScope) noexcept;

        static VKAPI_ATTR void VKAPI_CALL vklInternalFree(void *pUserData, std::size_t size, VkInternalAllocationType allocationType,
                                                          VkSystemAllocationScope allocationScope) noexcept;

        struct ScopeStats final {
            std::atomic<std::size_t> liveBytes{0};
            std::atomic<std::size_t> peakBytes{0};

            std::atomic<std::size_t> allocCount{0};
            std::atomic<std::size_t> freeCount{0};
            std::atomic<std::size_t> reallocCount{0};
            std::atomic<std::size_t> failCount{0};

            std::atomic<std::size_t> internalAllocCount{0};
            std::atomic<std::size_t> internalFreeCount{0};
            std::atomic<std::size_t> internalLiveBytes{0};
            std::atomic<std::size_t> internalPeakBytes{0};
        };

        [[nodiscard]] static constexpr std::size_t scopeIndex(VkSystemAllocationScope scope) noexcept {
            const auto v = static_cast<std::size_t>(std::to_underlying(scope));
            return (v < scopeCount) ? v : 0;
        }

        static void updatePeak(std::atomic<std::size_t> &peak, std::size_t candidate) noexcept;

        std::atomic<std::size_t> totalLiveBytes_{0};

        // Cumulativo delle allocazioni riuscite (diagnostica).
        std::atomic<std::size_t> totalCumulativeAllocatedBytes_{0};

        std::array<ScopeStats, scopeCount> scopes_{};
    };

}  // namespace vnd