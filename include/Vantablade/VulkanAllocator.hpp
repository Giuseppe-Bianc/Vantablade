/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

#include "Vantablade/headers.hpp"

DISABLE_WARNINGS_PUSH(4324)
namespace vnd {

#ifndef VND_ALLOCATOR_DEBUG
#define VND_ALLOCATOR_DEBUG 0
#endif

#ifndef VND_ALLOCATOR_STATS
#define VND_ALLOCATOR_STATS 1
#endif

#ifndef VND_ALLOCATOR_TELEMETRY
#define VND_ALLOCATOR_TELEMETRY 0
#endif

    static constexpr std::size_t scopeCount = 5;  // COMMAND, OBJECT, CACHE, DEVICE, INSTANCE

    // Compile-time validation: scopeCount must match Vulkan enum range
    static_assert(scopeCount == 5, "scopeCount must equal number of VkSystemAllocationScope values");
    static_assert(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE == 4, "Vulkan enum values changed; update scopeIndex mapping");

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

        // Not copyable or movable: prevents double-free and ensures singleton semantics
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

#if VND_ALLOCATOR_TELEMETRY
        using TelemetryCallback = void (*)(void *userData, const char *event, std::size_t size, VkSystemAllocationScope scope);
        void setTelemetryCallback(TelemetryCallback callback, void *userData = nullptr) noexcept {
            telemetryCallback.store(callback, std::memory_order_relaxed);
            telemetryUserData.store(userData, std::memory_order_relaxed);
        }
#endif

    private:
        struct alignas(alignof(void *)) AllocationHeader final {
            std::size_t size;               ///< Requested payload size
            void *base;                     ///< Pointer to start of malloc block
            VkSystemAllocationScope scope;  ///< Vulkan allocation scope
        };

        // Compile-time validation: header must fit within typical cache line
        static_assert(sizeof(AllocationHeader) <= 32, "AllocationHeader too large; impacts cache efficiency");
        static_assert(alignof(AllocationHeader) <= alignof(std::max_align_t), "AllocationHeader alignment exceeds platform max_align_t");

        struct alignas(64) ScopeStats final {  // 64B = typical cache line size
            std::atomic<std::size_t> liveBytes{0};
            std::atomic<std::size_t> peakBytes{0};
            std::atomic<std::size_t> allocCount{0};
            std::atomic<std::size_t> freeCount{0};
            std::atomic<std::size_t> reallocCount{0};
            std::atomic<std::size_t> failCount{0};
        };

        static_assert(alignof(ScopeStats) == 64, "ScopeStats must be cache-line aligned");

        [[nodiscard]] static constexpr std::size_t scopeIndex(VkSystemAllocationScope scope) noexcept {
            // C++23: std::to_underlying for enum class to integer conversion
            const auto v = static_cast<std::size_t>(std::to_underlying(scope));
            return (v < scopeCount) ? v : 0;
        }

        [[nodiscard]] static constexpr std::string_view scopeName(VkSystemAllocationScope scope) noexcept;

        static void updatePeak(std::atomic<std::size_t> &peak, std::size_t candidate) noexcept;

        [[nodiscard]] static constexpr std::size_t computeEffectiveAlignment(std::size_t alignment) noexcept {
            // Vulkan §11.8: alignment is power of two and > 0 [[assume]] communicates this
            return std::max(alignment, alignof(AllocationHeader));
        }

        [[nodiscard]] static constexpr bool checkedAdd(std::size_t lhs, std::size_t rhs, std::size_t &result) noexcept {
#if __cpp_lib_to_chars >= 202306L && defined(__has_builtin)
// C++23: prefer standard library if overflow detection available
#if __has_builtin(__builtin_add_overflow)
            return !__builtin_add_overflow(lhs, rhs, &result);
#else
            // Fallback: manual check (C++ Core Guidelines ES.103)
            if(rhs > std::numeric_limits<std::size_t>::max() - lhs) { return false; }
            result = lhs + rhs;
            return true;
#endif
#else
            // Portable fallback for all C++23 compilers
            if(rhs > std::numeric_limits<std::size_t>::max() - lhs) { return false; }
            result = lhs + rhs;
            return true;
#endif
        }

        [[nodiscard]] static constexpr bool isKnownScope(VkSystemAllocationScope scope) noexcept {
            const auto scopeValue = static_cast<std::size_t>(std::to_underlying(scope));
            return scopeValue < scopeCount;
        }

#if VND_ALLOCATOR_DEBUG
        static void poisonMemory(void *ptr, std::size_t size) noexcept {
            if(ptr != nullptr && size > 0) {
                std::memset(ptr, 0xDE, size);  // 0xDE = debug pattern for "dead" memory
            }
        }

        [[nodiscard]] static bool verifyMemory(void *ptr, std::size_t size) noexcept {
            if(ptr == nullptr || size == 0) { return true; }
            const auto *bytes = static_cast<const unsigned char *>(ptr);
            for(std::size_t i = 0; i < size; ++i) {
                if(bytes[i] != 0xDE) { return false; }
            }
            return true;
        }
#endif

        static VKAPI_ATTR void *VKAPI_CALL vklAllocation(void *pUserData, std::size_t size, std::size_t alignment,
                                                         VkSystemAllocationScope allocationScope) noexcept;

        static VKAPI_ATTR void *VKAPI_CALL vklReallocation(void *pUserData, void *pOriginal, std::size_t size, std::size_t alignment,
                                                           VkSystemAllocationScope allocationScope) noexcept;

        static VKAPI_ATTR void VKAPI_CALL vklFree(void *pUserData, void *pMemory) noexcept;

        void dumpOneScope(VkSystemAllocationScope scope) const;

        std::atomic<std::size_t> totalAllocated{0};   ///< Total live bytes across all scopes
        std::array<ScopeStats, scopeCount> scopes{};  ///< Per-scope statistics (cache-line aligned)

#if VND_ALLOCATOR_TELEMETRY
        std::atomic<TelemetryCallback> telemetryCallback{nullptr};
        std::atomic<void *> telemetryUserData{nullptr};
#endif
    };

}  // namespace vnd

DISABLE_WARNINGS_POP()