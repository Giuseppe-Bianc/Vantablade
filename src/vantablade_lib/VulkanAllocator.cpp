#include "Vantablade/VulkanAllocator.hpp"

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
            std::size_t size;
            void *base;
            VkSystemAllocationScope scope;
        };

        [[nodiscard]] constexpr std::size_t computeEffectiveAlignment(std::size_t alignment) noexcept {
            return std::max(alignment, alignof(AllocationHeader));
        }

        [[nodiscard]] constexpr bool checked_add(std::size_t a, std::size_t b, std::size_t &result) noexcept {
            if(b > std::numeric_limits<std::size_t>::max() - a) return false;
            result = a + b;
            return true;
        }

        [[nodiscard]] constexpr bool isKnownScope(VkSystemAllocationScope scope) noexcept {
            const auto v = static_cast<std::size_t>(std::to_underlying(scope));
            return v < scopeCount;
        }

    }  // namespace

    void VulkanAllocator::updatePeak(std::atomic<std::size_t> &peak, std::size_t candidate) noexcept {
        std::size_t old = peak.load(std::memory_order_relaxed);
        while(candidate > old && !peak.compare_exchange_weak(old, candidate, std::memory_order_relaxed, std::memory_order_relaxed)) {}
    }

    VulkanAllocator::ScopeSnapshot VulkanAllocator::getScopeSnapshot(VkSystemAllocationScope scope) const noexcept {
        const auto &s = scopes[scopeIndex(scope)];
        return ScopeSnapshot{
            .liveBytes = s.liveBytes.load(std::memory_order_relaxed),
            .peakBytes = s.peakBytes.load(std::memory_order_relaxed),
            .allocCount = s.allocCount.load(std::memory_order_relaxed),
            .freeCount = s.freeCount.load(std::memory_order_relaxed),
            .reallocCount = s.reallocCount.load(std::memory_order_relaxed),
            .failCount = s.failCount.load(std::memory_order_relaxed),
        };
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklAllocation(void *pUserData, std::size_t size, std::size_t alignment,
                                                               VkSystemAllocationScope allocationScope) noexcept {
        if(size == 0u) return nullptr;

        // Vulkan 1.4 §11.8 guarantees alignment is a power of two.
        // [[assume]] (C++23 P1774R8) communicates this invariant to the optimiser
        // without a runtime check; it has no effect in release if the compiler does
        // not act on it, and it is never evaluated as a runtime assertion.
        VND_ASSUME(alignment > 0u && (alignment & (alignment - 1u)) == 0u);

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        if(!isKnownScope(allocationScope)) {
            LERROR("Vulkan CPU allocation with unknown scope (scope={})", std::to_underlying(allocationScope));
        }

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
            self->scopes[scopeIndex(allocationScope)].failCount.fetch_add(1, std::memory_order_relaxed);
            LERROR("Vulkan CPU allocation overhead overflow (size={}, alignment={}, scope={})", size, alignment,
                   std::to_underlying(allocationScope));
            return nullptr;
        }

        std::size_t totalSize{};
        if(!checked_add(overhead, size, totalSize)) {
            self->scopes[scopeIndex(allocationScope)].failCount.fetch_add(1, std::memory_order_relaxed);
            LERROR("Vulkan CPU allocation totalSize overflow (size={}, alignment={}, scope={})", size, alignment,
                   std::to_underlying(allocationScope));
            return nullptr;
        }

        void *const base = std::malloc(totalSize);
        if(!base) {
            self->scopes[scopeIndex(allocationScope)].failCount.fetch_add(1, std::memory_order_relaxed);
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
        header->scope = allocationScope;

        self->totalAllocated.fetch_add(size, std::memory_order_relaxed);

        auto &st = self->scopes[scopeIndex(allocationScope)];
        st.allocCount.fetch_add(1, std::memory_order_relaxed);
        const std::size_t live = st.liveBytes.fetch_add(size, std::memory_order_relaxed) + size;
        updatePeak(st.peakBytes, live);

        return userPtr;
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklReallocation(void *pUserData, void *pOriginal, std::size_t size, std::size_t alignment,
                                                                 VkSystemAllocationScope allocationScope) noexcept {
        // Vulkan 1.4 §11.8: if pOriginal is NULL, behave as pfnAllocation.
        if(!pOriginal) { return vklAllocation(pUserData, size, alignment, allocationScope); }
        // Vulkan 1.4 §11.8: if size is 0, behave as pfnFree and return NULL.
        if(size == 0u) {
            vklFree(pUserData, pOriginal);
            return nullptr;
        }

        auto *const self = static_cast<VulkanAllocator *>(pUserData);
        self->scopes[scopeIndex(allocationScope)].reallocCount.fetch_add(1, std::memory_order_relaxed);
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
        const std::size_t sz = header->size;
        const VkSystemAllocationScope sc = header->scope;

        self->totalAllocated.fetch_sub(sz, std::memory_order_relaxed);

        auto &st = self->scopes[scopeIndex(sc)];
        st.freeCount.fetch_add(1, std::memory_order_relaxed);
        st.liveBytes.fetch_sub(sz, std::memory_order_relaxed);

        std::free(header->base);
    }

    [[nodiscard]] constexpr std::string_view vnd::VulkanAllocator::scopeName(VkSystemAllocationScope scope) noexcept {
        switch(scope) {
        case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND:
            return "COMMAND";
        case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT:
            return "OBJECT";
        case VK_SYSTEM_ALLOCATION_SCOPE_CACHE:
            return "CACHE";
        case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE:
            return "DEVICE";
        case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE:
            return "INSTANCE";
        default:
            return "UNKNOWN";
        }
    }

    void vnd::VulkanAllocator::dumpOneScope(VkSystemAllocationScope scope) const {
        const auto s = getScopeSnapshot(scope);

        LINFO("[{}] live={} peak={} alloc={} free={} realloc={} fail={}", scopeName(scope), s.liveBytes, s.peakBytes, s.allocCount,
              s.freeCount, s.reallocCount, s.failCount);
    }

    void vnd::VulkanAllocator::dumpReport() const {
        const std::size_t totalLive = getTotalAllocated();

        LINFO("VulkanAllocator CPU Memory Report");

        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_CACHE);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

        const std::size_t s0 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND).liveBytes;
        const std::size_t s1 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT).liveBytes;
        const std::size_t s2 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_CACHE).liveBytes;
        const std::size_t s3 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE).liveBytes;
        const std::size_t s4 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE).liveBytes;

        const std::size_t scopesSum = s0 + s1 + s2 + s3 + s4;
        const std::size_t delta = (scopesSum >= totalLive) ? (scopesSum - totalLive) : (totalLive - scopesSum);

        LINFO("total_live={}|-|scopes_sum={}|-|delta={}", totalLive, scopesSum, delta);
    }

}  // namespace vnd