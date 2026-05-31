#pragma once

#include "Vantablade/headers.hpp"
#include "Vantablade/vulkanCheck.hpp"

#ifdef VANTABLADE_PROFILING
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>
#endif

namespace Vantablade {

// ---------------------------------------------------------------------------
// CPU profiling macros. These were correct in the original and are unchanged.
// ---------------------------------------------------------------------------
#ifdef VANTABLADE_PROFILING
#define VZ_ZONE_SCOPED ZoneScoped
#define VZ_ZONE_SCOPED_NAMED(name) ZoneScopedN(name)
#define VZ_FRAME_MARK() FrameMark
#define VZ_PLOT_INT(name, value) TracyPlot(name, static_cast<int64_t>(value))
#define VZ_PLOT_FLOAT(name, value) TracyPlot(name, static_cast<float>(value))
#else
#define VZ_ZONE_SCOPED ((void)0)
#define VZ_ZONE_SCOPED_NAMED(name) ((void)0)
#define VZ_FRAME_MARK() ((void)0)
#define VZ_PLOT_INT(name, value) ((void)0)
#define VZ_PLOT_FLOAT(name, value) ((void)0)
#endif

// ---------------------------------------------------------------------------
// GPU profiling macros.
//
// VZ_GPU_ZONE(ctx, cmd, "name")
//   Creates a RAII Tracy GPU zone on cmd. Ends at enclosing scope exit.
//   ctx must be a TracyVkCtx obtained from VulkanProfiler::getContext().
//   cmd must be a VkCommandBuffer in recording state.
//   This must remain a macro: Tracy's zone macros capture __FILE__ and __LINE__
//   at the call site. A class wrapper cannot do this without losing location.
//
// VZ_GPU_COLLECT(ctx, cmd)
//   Records timestamp retrieval commands into cmd. Call once per frame,
//   at the end of the frame's command buffer before vkEndCommandBuffer.
// ---------------------------------------------------------------------------
#ifdef VANTABLADE_PROFILING
#define VZ_GPU_ZONE(ctx, cmd, name) TracyVkZone(ctx, cmd, name)
#define VZ_GPU_COLLECT(ctx, cmd) TracyVkCollect(ctx, cmd)
#else
#define VZ_GPU_ZONE(ctx, cmd, name) ((void)0)
#define VZ_GPU_COLLECT(ctx, cmd) ((void)0)
#endif

// ---------------------------------------------------------------------------
// VulkanProfiler
//
// Owns a single Tracy Vulkan context. One instance per queue.
// If you profile multiple queues, create one VulkanProfiler per queue.
//
// Lifecycle:
//   1. init(params)         after vkCreateDevice, with a cmd buffer in recording state.
//   2. collect(cmd)         every frame, at the end of that frame's cmd buffer.
//   3. shutdown()           before vkDestroyDevice, with GPU idle.
//                           Also called automatically by the destructor.
// ---------------------------------------------------------------------------
#ifdef VANTABLADE_PROFILING

    class VulkanProfiler {
    public:
        /**
         * Parameters for Tracy Vulkan context initialisation.
         *
         * cmdBuffer must be in recording state when passed to init().
         * After init() returns, the caller must:
         *   1. vkEndCommandBuffer(params.cmdBuffer)
         *   2. vkQueueSubmit to params.queue
         *   3. vkQueueWaitIdle (or fence wait)
         * Tracy writes calibration timestamp queries into cmdBuffer during init.
         */
        struct InitParams {
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            VkQueue queue = VK_NULL_HANDLE;
            VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;  // Must be in recording state.
            const char *contextName = "GPU";
        };

        VulkanProfiler() = default;

        // Calls shutdown() if the context is still alive.
        ~VulkanProfiler();

        VulkanProfiler(const VulkanProfiler &) = delete;
        VulkanProfiler &operator=(const VulkanProfiler &) = delete;
        VulkanProfiler(VulkanProfiler &&) = delete;
        VulkanProfiler &operator=(VulkanProfiler &&) = delete;

        /**
         * Creates the Tracy Vulkan context.
         *
         * Preconditions:
         *   - All params fields are valid and non-null.
         *   - params.cmdBuffer is in recording state.
         *   - params.queue supports timestamps
         *     (VkQueueFamilyProperties::timestampValidBits > 0).
         *
         * Cost: one-time init, not called in the hot path.
         */
        void init(const InitParams &params);

        /**
         * Destroys the Tracy Vulkan context.
         *
         * Precondition: GPU must be idle. Call vkDeviceWaitIdle before this.
         * Safe to call on an uninitialised profiler or after prior shutdown().
         */
        void shutdown();

        /**
         * Records timestamp collection commands into cmd.
         *
         * Call once per frame at the end of the frame's primary command buffer,
         * before vkEndCommandBuffer. Tracy uses these commands to read back
         * GPU timestamps and correlate them with the CPU timeline.
         *
         * Preconditions:
         *   - cmd must be in recording state.
         *   - Previous frame's GPU work must be complete (fence signalled).
         *
         * Cost: records a small number of Vulkan commands, no CPU stall.
         */
        void collect(VkCommandBuffer cmd);

        /**
         * Returns the underlying Tracy Vulkan context handle.
         *
         * Pass to VZ_GPU_ZONE so Tracy can capture source location at the call site:
         *   VZ_GPU_ZONE(m_profiler.getContext(), cmd, "Shadow Pass");
         *
         * Never cache this value across init/shutdown cycles.
         */
        [[nodiscard]] TracyVkCtx getContext() const noexcept { return m_tracyCtx; }

        [[nodiscard]] bool isInitialized() const noexcept { return m_tracyCtx != nullptr; }

    private:
        TracyVkCtx m_tracyCtx = nullptr;
    };

#else  // VANTABLADE_PROFILING not defined: zero-cost no-ops, same public interface.

    class VulkanProfiler {
    public:
        struct InitParams {
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            VkQueue queue = VK_NULL_HANDLE;
            VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
            const char *contextName = nullptr;
        };

        VulkanProfiler() = default;
        ~VulkanProfiler() = default;
        VulkanProfiler(const VulkanProfiler &) = delete;
        VulkanProfiler &operator=(const VulkanProfiler &) = delete;

        void init(const InitParams &) {}
        void shutdown() {}
        void collect(VkCommandBuffer) {}
        // getContext() is intentionally absent: VZ_GPU_ZONE expands to ((void)0)
        // and never evaluates its arguments, so the call site never reaches getContext().
        [[nodiscard]] bool isInitialized() const noexcept { return false; }
    };

#endif  // VANTABLADE_PROFILING

}  // namespace Vantablade