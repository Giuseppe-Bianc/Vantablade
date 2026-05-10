/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
#include "Vantablade/VulkanAllocator.hpp"

// Portable assume hint.
//
// [[assume(expr)]] is C++23 (P1774R8); __has_cpp_attribute detects real support.
// Falls back to compiler-specific intrinsics on older toolchains:
//   MSVC  : __assume(expr)          — recognised since VS 2005
//   Clang : __builtin_assume(expr)  — recognised since Clang 3.6
//   GCC   : if (!(expr)) __builtin_unreachable() — GCC has no direct assume,
//           but unreachable on the false branch is semantically equivalent.
#if __has_cpp_attribute(assume) >= 202207L
#define VND_ASSUME(expr) [[assume(expr)]]
#elif defined(_MSC_VER)
#define VND_ASSUME(expr) __assume(expr)
#elif defined(__clang__)
#define VND_ASSUME(expr) __builtin_assume(expr)
#elif defined(__GNUC__)
#define VND_ASSUME(expr)                                                                                                                   \
    do {                                                                                                                                   \
        if(!(expr)) __builtin_unreachable();                                                                                               \
    } while(0)
#else
#define VND_ASSUME(expr) ((void)0)
#endif

namespace vnd {

    namespace {

        struct alignas(alignof(void *)) AllocationHeader final {
            std::size_t size;  // user-requested bytes; used to update the live counter on free
            void *base;        // original std::malloc base; passed to std::free on deallocation
        };

        // Returns the smallest alignment satisfying both the Vulkan-requested alignment
        // and the alignment required to store an AllocationHeader immediately before the
        // user pointer.
        [[nodiscard]] constexpr std::size_t computeEffectiveAlignment(std::size_t alignment) noexcept {
            return std::max(alignment, alignof(AllocationHeader));
        }

        /**
         * Portable checked addition for std::size_t (C++23).
         *
         * Returns true and writes the sum to `result` if a + b does not overflow.
         * Returns false and leaves `result` unchanged if it would overflow.
         *
         * Identity: a + b > SIZE_MAX  <=>  b > SIZE_MAX - a
         * Safe to evaluate because SIZE_MAX - a never underflows (a <= SIZE_MAX).
         *
         * C++26 will provide std::add_sat / std::overflow_detection for this purpose.
         * __builtin_add_overflow is available on GCC and Clang but is not ISO standard.
         * This implementation is fully portable per C++23 (ES.103, C++ Core Guidelines).
         */
        [[nodiscard]] constexpr bool checked_add(std::size_t a, std::size_t b, std::size_t &result) noexcept {
            if(b > std::numeric_limits<std::size_t>::max() - a) return false;
            result = a + b;
            return true;
        }

    }  // namespace

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklAllocation(void *pUserData, std::size_t size, std::size_t alignment,
                                                               VkSystemAllocationScope allocationScope) noexcept {
        if(size == 0u) return nullptr;

        // Vulkan 1.4 §11.8 guarantees alignment is a power of two.
        // [[assume]] (C++23 P1774R8) communicates this invariant to the optimiser
        // without a runtime check; it has no effect in release if the compiler does
        // not act on it, and it is never evaluated as a runtime assertion.
        VND_ASSUME(alignment > 0u && (alignment & (alignment - 1u)) == 0u);

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        const std::size_t effectiveAlign = computeEffectiveAlignment(alignment);

        // Total bytes needed for the malloc block:
        //   sizeof(AllocationHeader)   — header placed immediately before the user pointer
        //   + (effectiveAlign - 1)     — worst-case padding to satisfy effectiveAlign
        //   + size                     — user-requested payload
        //
        // effectiveAlign >= alignof(AllocationHeader) >= alignof(void*) >= 4, so
        // effectiveAlign - 1u cannot underflow.  Both additions are checked independently;
        // correctness must not depend on Vulkan drivers never requesting allocations
        // near SIZE_MAX (C++ Core Guidelines ES.103).
        std::size_t overhead{};
        if(!checked_add(sizeof(AllocationHeader), effectiveAlign - 1u, overhead)) {
            LERROR("Vulkan CPU allocation overhead overflow (size={}, alignment={}, scope={})", size, alignment,
                   std::to_underlying(allocationScope));
            return nullptr;
        }

        std::size_t totalSize{};
        if(!checked_add(overhead, size, totalSize)) {
            LERROR("Vulkan CPU allocation totalSize overflow (size={}, alignment={}, scope={})", size, alignment,
                   std::to_underlying(allocationScope));
            return nullptr;
        }

        void *const base = std::malloc(totalSize);
        if(!base) {
            LERROR("Vulkan CPU allocation failed (size={}, alignment={}, scope={})", size, alignment, std::to_underlying(allocationScope));
            return nullptr;
        }

        // Advance past the header to find the first effectiveAlign-aligned address.
        //
        // std::align (<memory>, C++11) is the standard mechanism for computing aligned
        // sub-regions within an existing buffer.  It advances `userPtr` forward by at
        // most (effectiveAlign - 1) bytes and reduces `space` accordingly, then returns
        // the aligned pointer (or nullptr if space is insufficient).
        //
        // Failure is impossible here: `totalSize` was computed to guarantee at least
        // sizeof(AllocationHeader) + (effectiveAlign - 1) + size bytes.  The assert
        // documents this invariant and fires in debug builds if the invariant is ever
        // violated by a future refactor.
        void *userPtr = static_cast<char *>(base) + sizeof(AllocationHeader);
        std::size_t space = totalSize - sizeof(AllocationHeader);

        [[maybe_unused]] void *const alignResult = std::align(effectiveAlign, size, userPtr, space);
        assert(alignResult != nullptr);  // cannot fail: totalSize guarantees sufficient room

        // Place the header in the sizeof(AllocationHeader) bytes immediately before the
        // aligned user pointer.  This region is always within the malloc block:
        //   userPtr >= base + sizeof(AllocationHeader)
        //   => userPtr - sizeof(AllocationHeader) >= base  (valid, within the block)
        //
        // AllocationHeader is an implicit-lifetime type (trivially constructible and
        // trivially destructible).  Its lifetime begins when the malloc block is created
        // (C++23 [intro.object] §6.7.2 p10), so no placement-new is required.
        auto *const header = reinterpret_cast<AllocationHeader *>(static_cast<char *>(userPtr) - sizeof(AllocationHeader));
        header->size = size;
        header->base = base;

        self->totalAllocated.fetch_add(size, std::memory_order_relaxed);
        return userPtr;
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklReallocation(void *pUserData, void *pOriginal, std::size_t size, std::size_t alignment,
                                                                 VkSystemAllocationScope allocationScope) noexcept {
        // Vulkan 1.4 §11.8: if pOriginal is NULL, behave as pfnAllocation.
        if(!pOriginal){ return vklAllocation(pUserData, size, alignment, allocationScope);}
        // Vulkan 1.4 §11.8: if size is 0, behave as pfnFree and return NULL.
        if(size == 0u) {
            vklFree(pUserData, pOriginal);
            return nullptr;
        }

        // Recover the old live size from the header immediately before the user pointer.
        // Pointer arithmetic on char* is well-defined within the same malloc block
        // (C++ [expr.add] §7.6.6 p4); no uintptr_t round-trip is needed.
        const auto *const oldHeader = reinterpret_cast<const AllocationHeader *>(static_cast<char *>(pOriginal) - sizeof(AllocationHeader));
        const std::size_t oldSize = oldHeader->size;

        void *const newPtr = vklAllocation(pUserData, size, alignment, allocationScope);
        if(newPtr) {
            // Old and new blocks come from independent malloc calls; overlap is impossible.
            // Copy only the bytes valid in both buffers per Vulkan 1.4 §11.8.
            std::memcpy(newPtr, pOriginal, std::ranges::min(oldSize, size));
            vklFree(pUserData, pOriginal);
        }
        // If vklAllocation failed, pOriginal remains valid.
        // Vulkan 1.4 §11.8: "if this function fails and pOriginal is non-NULL,
        // the application must not free the old allocation."
        return newPtr;
    }

    VKAPI_ATTR void VKAPI_CALL VulkanAllocator::vklFree(void *pUserData, void *pMemory) noexcept {
        // Vulkan 1.4 §11.8: pMemory == NULL must be a no-op.
        if(!pMemory) { return; }

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        // Recover the header from the sizeof(AllocationHeader) bytes before the user pointer.
        // char* arithmetic stays within the original malloc block; no uintptr_t round-trip.
        const auto *const header = reinterpret_cast<const AllocationHeader *>(static_cast<char *>(pMemory) - sizeof(AllocationHeader));

        self->totalAllocated.fetch_sub(header->size, std::memory_order_relaxed);
        std::free(header->base);
    }

}  // namespace vnd