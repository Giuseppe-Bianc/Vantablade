// TRACY_IMPLEMENTATION must be defined before the first inclusion of any Tracy
// header in this translation unit. Profiler.hpp includes Tracy.hpp transitively,
// so this define must appear before the Profiler.hpp include below.
//
// This makes the current translation unit own Tracy's compiled implementation.
// Exactly one TU in the program may define this.
//
// Alternative (preferred for larger projects): remove this define entirely and
// instead add ${TRACY_DIR}/public/TracyClient.cpp to your CMake target_sources.
// That file handles everything and removes the ordering constraint.
#ifdef VANTABLADE_PROFILING
#define TRACY_IMPLEMENTATION
#endif

#include "Vantablade/Profiler.hpp"

namespace Vantablade {

#ifdef VANTABLADE_PROFILING

    VulkanProfiler::~VulkanProfiler() { shutdown(); }

    void VulkanProfiler::init(const InitParams &params) {
        // TracyVkContext creates Tracy's internal query pool and records its own
        // initialization commands into params.cmdBuffer. The command buffer must
        // be valid, but the caller does not begin or end it around this call.
        m_tracyCtx = TracyVkContext(params.physicalDevice, params.device, params.queue, params.cmdBuffer);

        // If VK_EXT_calibrated_timestamps is available on the target hardware,
        // replace the line above with:
        //
        //   auto getCalibrateableTimeDomains =
        //       reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>(
        //           vkGetDeviceProcAddr(params.device,
        //               "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT"));
        //   auto getCalibratedTimestamps =
        //       reinterpret_cast<PFN_vkGetCalibratedTimestampsEXT>(
        //           vkGetDeviceProcAddr(params.device,
        //               "vkGetCalibratedTimestampsEXT"));
        //
        //   m_tracyCtx = TracyVkContextCalibrated(
        //       params.physicalDevice,
        //       params.device,
        //       params.queue,
        //       params.cmdBuffer,
        //       getCalibrateableTimeDomains,
        //       getCalibratedTimestamps
        //   );
        //
        // The calibrated variant produces significantly more accurate CPU/GPU
        // timeline correlation on hardware that supports it (most modern AMD/Intel,
        // and Vulkan 1.2+ NVIDIA drivers on Linux/Windows 10+).

        if(m_tracyCtx && params.contextName) {
            const auto len = static_cast<uint16_t>(std::strlen(params.contextName));
            TracyVkContextName(m_tracyCtx, params.contextName, len);
        }
    }

    void VulkanProfiler::shutdown() {
        if(!m_tracyCtx) { return; }
        // GPU must be idle before this call. TracyVkDestroy reads back any pending
        // timestamp queries and frees Tracy's internal Vulkan resources.
        TracyVkDestroy(m_tracyCtx);
        m_tracyCtx = nullptr;
    }

    void VulkanProfiler::collect(VkCommandBuffer cmd) {
        if(!m_tracyCtx) { return; }
        // Records commands into cmd that will retrieve timestamp results when the
        // GPU executes this command buffer. No CPU stall. Results are forwarded
        // to Tracy asynchronously after the GPU completes the frame.
        TracyVkCollect(m_tracyCtx, cmd);
    }

#endif  // VANTABLADE_PROFILING

}  // namespace Vantablade
