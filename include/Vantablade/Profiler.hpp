#pragma once

#include "Vantablade/headers.hpp"
#include "Vantablade/vulkanCheck.hpp"

#ifdef VANTABLADE_PROFILING
#include <tracy/Tracy.hpp>
#endif

namespace Vantablade {

/**
 * @brief Zero-cost profiling wrapper for Tracy.
 * Resolves to NOPs when VANTABLADE_PROFILING is not defined.
 */
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

#ifdef VANTABLADE_PROFILING
    class VulkanProfiler {
    public:
        struct GpuZone {
            uint32_t startQuery;
            uint32_t endQuery;
            const char *name;
        };

        VulkanProfiler() = default;
        ~VulkanProfiler();

        void init(VkDevice device, VkPhysicalDevice physicalDevice);
        void shutdown();
        void mapQueue(uint32_t queueIndex, VkQueue queue);

        void beginGpuZone(VkCommandBuffer cmd, const char *name);
        void endGpuZone(VkCommandBuffer cmd);
        void resolveTimestamps();

    private:
        VkDevice m_device = VK_NULL_HANDLE;
        VkQueryPool m_queryPool = VK_NULL_HANDLE;
        float m_timestampPeriod = 1.0f;
        uint32_t m_currentQueryIndex = 0;

        std::vector<GpuZone> m_activeZones;
    };
#else
    class VulkanProfiler {
    public:
        VulkanProfiler() = default;
        ~VulkanProfiler() = default;
        // cppcheck-suppress functionStatic
        void init(VkDevice, VkPhysicalDevice) {}
        // cppcheck-suppress functionStatic
        void shutdown() {}
        // cppcheck-suppress functionStatic
        void mapQueue(uint32_t, VkQueue) {}
        // cppcheck-suppress functionStatic
        void beginGpuZone(VkCommandBuffer, const char *) {}
        // cppcheck-suppress functionStatic
        void endGpuZone(VkCommandBuffer) {}
        // cppcheck-suppress functionStatic
        void resolveTimestamps() {}
    };
#endif

}  // namespace Vantablade
