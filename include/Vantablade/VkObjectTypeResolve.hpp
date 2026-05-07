/*
 * Created by gbian on 04/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
// ─── Section 1: File Header ──────────────────────────────────────────────────
// Purpose   : Compile-time mapping from Vulkan handle types to their
//             corresponding VkObjectType enumeration values.
// Standard  : C++23 or later (uses constexpr, if constexpr, static_assert).
// Dependency: <vulkan/vulkan.h> must be available on the include path.
//             Link against a Vulkan loader that exposes VK_EXT_debug_utils
//             for the setVulkanDebugName usage example.
//
// Coverage  : Every VkObjectType enumerator defined in the Vulkan 1.3 core
//             specification plus the following ratified extensions:
//               • VK_KHR_swapchain
//               • VK_KHR_display / VK_KHR_display_swapchain
//               • VK_KHR_video_queue
//               • VK_KHR_deferred_host_operations
//               • VK_KHR_acceleration_structure
//               • VK_EXT_debug_report (deprecated but still present)
//               • VK_EXT_debug_utils
//               • VK_EXT_validation_cache
//               • VK_EXT_private_data
//               • VK_EXT_opacity_micromap
//               • VK_NVX_binary_import
//               • VK_NV_optical_flow
//               • VK_NV_ray_tracing (legacy)
//               • VK_INTEL_performance_query
//               • VK_FUCHSIA_buffer_collection

// ─── Section 2: Includes and Namespace ───────────────────────────────────────
#include "headers.hpp"

namespace vkutil {

    // ─── Section 3: Primary Template Declaration ─────────────────────────────────
    // The primary template is intentionally left undefined for all unsupported
    // types. The static_assert inside fires an informative diagnostic message
    // rather than an obscure linker error when an unsupported handle is used.
    //
    // Design choice: function template rather than a type-trait struct so that
    // call sites read as plain function calls: vulkanObjectType<VkBuffer>().

    template <typename T> constexpr VkObjectType vulkanObjectType() noexcept {
        static_assert(!std::is_same_v<T, T>,  // Always false; evaluated only on instantiation.
                      "vulkanObjectType<T>: T is not a recognised Vulkan handle type. "
                      "Add a specialization in Section 4 to support this type.");
        return VK_OBJECT_TYPE_UNKNOWN;  // Suppresses "no return" warnings.
    }

    // ─── Section 4: Specializations ──────────────────────────────────────────────
    // Ordered to follow the numeric value of the VkObjectType enumerator so that
    // additions are easy to locate and verify against the specification table.
    //
    // ── 4.1  Core Vulkan 1.0 / 1.1 / 1.2 / 1.3 ──────────────────────────────────

    // VK_OBJECT_TYPE_UNKNOWN == 0
    // No handle maps to UNKNOWN; it is a sentinel for "not set".

    // VK_OBJECT_TYPE_INSTANCE == 1
    template <> constexpr VkObjectType vulkanObjectType<VkInstance>() noexcept { return VK_OBJECT_TYPE_INSTANCE; }

    // VK_OBJECT_TYPE_PHYSICAL_DEVICE == 2
    template <> constexpr VkObjectType vulkanObjectType<VkPhysicalDevice>() noexcept { return VK_OBJECT_TYPE_PHYSICAL_DEVICE; }

    // VK_OBJECT_TYPE_DEVICE == 3
    template <> constexpr VkObjectType vulkanObjectType<VkDevice>() noexcept { return VK_OBJECT_TYPE_DEVICE; }

    // VK_OBJECT_TYPE_QUEUE == 4
    template <> constexpr VkObjectType vulkanObjectType<VkQueue>() noexcept { return VK_OBJECT_TYPE_QUEUE; }

    // VK_OBJECT_TYPE_SEMAPHORE == 5
    template <> constexpr VkObjectType vulkanObjectType<VkSemaphore>() noexcept { return VK_OBJECT_TYPE_SEMAPHORE; }

    // VK_OBJECT_TYPE_COMMAND_BUFFER == 6
    template <> constexpr VkObjectType vulkanObjectType<VkCommandBuffer>() noexcept { return VK_OBJECT_TYPE_COMMAND_BUFFER; }

    // VK_OBJECT_TYPE_FENCE == 7
    template <> constexpr VkObjectType vulkanObjectType<VkFence>() noexcept { return VK_OBJECT_TYPE_FENCE; }

    // VK_OBJECT_TYPE_DEVICE_MEMORY == 8
    template <> constexpr VkObjectType vulkanObjectType<VkDeviceMemory>() noexcept { return VK_OBJECT_TYPE_DEVICE_MEMORY; }

    // VK_OBJECT_TYPE_BUFFER == 9
    template <> constexpr VkObjectType vulkanObjectType<VkBuffer>() noexcept { return VK_OBJECT_TYPE_BUFFER; }

    // VK_OBJECT_TYPE_IMAGE == 10
    template <> constexpr VkObjectType vulkanObjectType<VkImage>() noexcept { return VK_OBJECT_TYPE_IMAGE; }

    // VK_OBJECT_TYPE_EVENT == 11
    template <> constexpr VkObjectType vulkanObjectType<VkEvent>() noexcept { return VK_OBJECT_TYPE_EVENT; }

    // VK_OBJECT_TYPE_QUERY_POOL == 12
    template <> constexpr VkObjectType vulkanObjectType<VkQueryPool>() noexcept { return VK_OBJECT_TYPE_QUERY_POOL; }

    // VK_OBJECT_TYPE_BUFFER_VIEW == 13
    template <> constexpr VkObjectType vulkanObjectType<VkBufferView>() noexcept { return VK_OBJECT_TYPE_BUFFER_VIEW; }

    // VK_OBJECT_TYPE_IMAGE_VIEW == 14
    template <> constexpr VkObjectType vulkanObjectType<VkImageView>() noexcept { return VK_OBJECT_TYPE_IMAGE_VIEW; }

    // VK_OBJECT_TYPE_SHADER_MODULE == 15
    // Deprecated in Vulkan 1.3 (superseded by VK_EXT_shader_object) but the
    // handle type and object-type enumerator remain in the specification.
    template <> constexpr VkObjectType vulkanObjectType<VkShaderModule>() noexcept { return VK_OBJECT_TYPE_SHADER_MODULE; }

    // VK_OBJECT_TYPE_PIPELINE_CACHE == 16
    template <> constexpr VkObjectType vulkanObjectType<VkPipelineCache>() noexcept { return VK_OBJECT_TYPE_PIPELINE_CACHE; }

    // VK_OBJECT_TYPE_PIPELINE_LAYOUT == 17
    template <> constexpr VkObjectType vulkanObjectType<VkPipelineLayout>() noexcept { return VK_OBJECT_TYPE_PIPELINE_LAYOUT; }

    // VK_OBJECT_TYPE_RENDER_PASS == 18
    template <> constexpr VkObjectType vulkanObjectType<VkRenderPass>() noexcept { return VK_OBJECT_TYPE_RENDER_PASS; }

    // VK_OBJECT_TYPE_PIPELINE == 19
    template <> constexpr VkObjectType vulkanObjectType<VkPipeline>() noexcept { return VK_OBJECT_TYPE_PIPELINE; }

    // VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT == 20
    template <> constexpr VkObjectType vulkanObjectType<VkDescriptorSetLayout>() noexcept { return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; }

    // VK_OBJECT_TYPE_SAMPLER == 21
    template <> constexpr VkObjectType vulkanObjectType<VkSampler>() noexcept { return VK_OBJECT_TYPE_SAMPLER; }

    // VK_OBJECT_TYPE_DESCRIPTOR_POOL == 22
    template <> constexpr VkObjectType vulkanObjectType<VkDescriptorPool>() noexcept { return VK_OBJECT_TYPE_DESCRIPTOR_POOL; }

    // VK_OBJECT_TYPE_DESCRIPTOR_SET == 23
    template <> constexpr VkObjectType vulkanObjectType<VkDescriptorSet>() noexcept { return VK_OBJECT_TYPE_DESCRIPTOR_SET; }

    // VK_OBJECT_TYPE_FRAMEBUFFER == 24
    template <> constexpr VkObjectType vulkanObjectType<VkFramebuffer>() noexcept { return VK_OBJECT_TYPE_FRAMEBUFFER; }

    // VK_OBJECT_TYPE_COMMAND_POOL == 25
    template <> constexpr VkObjectType vulkanObjectType<VkCommandPool>() noexcept { return VK_OBJECT_TYPE_COMMAND_POOL; }

    // ── 4.2  Vulkan 1.1 core promotions ──────────────────────────────────────────

    // VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION == 1000156000
    // Promoted from VK_KHR_sampler_ycbcr_conversion.
    template <> constexpr VkObjectType vulkanObjectType<VkSamplerYcbcrConversion>() noexcept {
        return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION;
    }

    // VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE == 1000085000
    // Promoted from VK_KHR_descriptor_update_template.
    template <> constexpr VkObjectType vulkanObjectType<VkDescriptorUpdateTemplate>() noexcept {
        return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE;
    }

    // ── 4.3  Vulkan 1.3 core promotions ──────────────────────────────────────────

    // VK_OBJECT_TYPE_PRIVATE_DATA_SLOT == 1000295000
    // Promoted from VK_EXT_private_data.
    template <> constexpr VkObjectType vulkanObjectType<VkPrivateDataSlot>() noexcept { return VK_OBJECT_TYPE_PRIVATE_DATA_SLOT; }

    // ── 4.4  KHR extensions ───────────────────────────────────────────────────────

    // VK_OBJECT_TYPE_SWAPCHAIN_KHR == 1000001000
    // VK_KHR_swapchain
    template <> constexpr VkObjectType vulkanObjectType<VkSwapchainKHR>() noexcept { return VK_OBJECT_TYPE_SWAPCHAIN_KHR; }

    // VK_OBJECT_TYPE_DISPLAY_KHR == 1000002000
    // VK_KHR_display
    template <> constexpr VkObjectType vulkanObjectType<VkDisplayKHR>() noexcept { return VK_OBJECT_TYPE_DISPLAY_KHR; }

    // VK_OBJECT_TYPE_DISPLAY_MODE_KHR == 1000002001
    // VK_KHR_display
    template <> constexpr VkObjectType vulkanObjectType<VkDisplayModeKHR>() noexcept { return VK_OBJECT_TYPE_DISPLAY_MODE_KHR; }

// VK_OBJECT_TYPE_VIDEO_SESSION_KHR == 1000023000
// VK_KHR_video_queue
#if defined(VK_KHR_video_queue)
    template <> constexpr VkObjectType vulkanObjectType<VkVideoSessionKHR>() noexcept { return VK_OBJECT_TYPE_VIDEO_SESSION_KHR; }
#endif

// VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR == 1000023001
// VK_KHR_video_queue
#if defined(VK_KHR_video_queue)
    template <> constexpr VkObjectType vulkanObjectType<VkVideoSessionParametersKHR>() noexcept {
        return VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR;
    }
#endif

// VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR == 1000268000
// VK_KHR_deferred_host_operations
#if defined(VK_KHR_deferred_host_operations)
    template <> constexpr VkObjectType vulkanObjectType<VkDeferredOperationKHR>() noexcept { return VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR; }
#endif

// VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR == 1000150000
// VK_KHR_acceleration_structure
#if defined(VK_KHR_acceleration_structure)
    template <> constexpr VkObjectType vulkanObjectType<VkAccelerationStructureKHR>() noexcept {
        return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
    }
#endif

// ── 4.5  EXT extensions ───────────────────────────────────────────────────────

// VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT == 1000011000
// VK_EXT_debug_report (deprecated, superseded by VK_EXT_debug_utils)
#if defined(VK_EXT_debug_report)
    template <> constexpr VkObjectType vulkanObjectType<VkDebugReportCallbackEXT>() noexcept {
        return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
    }
#endif

// VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT == 1000128000
// VK_EXT_debug_utils
#if defined(VK_EXT_debug_utils)
    template <> constexpr VkObjectType vulkanObjectType<VkDebugUtilsMessengerEXT>() noexcept {
        return VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT;
    }
#endif

// VK_OBJECT_TYPE_VALIDATION_CACHE_EXT == 1000160000
// VK_EXT_validation_cache
#if defined(VK_EXT_validation_cache)
    template <> constexpr VkObjectType vulkanObjectType<VkValidationCacheEXT>() noexcept { return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; }
#endif

// VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV == 1000165000
// VK_NV_ray_tracing (legacy, predates KHR variant)
#if defined(VK_NV_ray_tracing)
    template <> constexpr VkObjectType vulkanObjectType<VkAccelerationStructureNV>() noexcept {
        return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV;
    }
#endif

// VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL == 1000210000
// VK_INTEL_performance_query
#if defined(VK_INTEL_performance_query)
    template <> constexpr VkObjectType vulkanObjectType<VkPerformanceConfigurationINTEL>() noexcept {
        return VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL;
    }
#endif

// VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV == 1000277000
// VK_NV_device_generated_commands
#if defined(VK_NV_device_generated_commands)
    template <> constexpr VkObjectType vulkanObjectType<VkIndirectCommandsLayoutNV>() noexcept {
        return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV;
    }
#endif

// VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA == 1000366000
// VK_FUCHSIA_buffer_collection
#if defined(VK_FUCHSIA_buffer_collection)
    template <> constexpr VkObjectType vulkanObjectType<VkBufferCollectionFUCHSIA>() noexcept {
        return VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA;
    }
#endif

// VK_OBJECT_TYPE_MICROMAP_EXT == 1000396000
// VK_EXT_opacity_micromap
#if defined(VK_EXT_opacity_micromap)
    template <> constexpr VkObjectType vulkanObjectType<VkMicromapEXT>() noexcept { return VK_OBJECT_TYPE_MICROMAP_EXT; }
#endif

// VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV == 1000464000
// VK_NV_optical_flow
#if defined(VK_NV_optical_flow)
    template <> constexpr VkObjectType vulkanObjectType<VkOpticalFlowSessionNV>() noexcept {
        return VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV;
    }
#endif

// VK_OBJECT_TYPE_SHADER_EXT == 1000482000
// VK_EXT_shader_object
#if defined(VK_EXT_shader_object)
    template <> constexpr VkObjectType vulkanObjectType<VkShaderEXT>() noexcept { return VK_OBJECT_TYPE_SHADER_EXT; }
#endif

// VK_OBJECT_TYPE_PIPELINE_BINARY_KHR == 1000483000
// VK_KHR_pipeline_binary
#if defined(VK_KHR_pipeline_binary)
    template <> constexpr VkObjectType vulkanObjectType<VkPipelineBinaryKHR>() noexcept { return VK_OBJECT_TYPE_PIPELINE_BINARY_KHR; }
#endif

// VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT == 1000572000
// VK_EXT_device_generated_commands
#if defined(VK_EXT_device_generated_commands)
    template <> constexpr VkObjectType vulkanObjectType<VkIndirectCommandsLayoutEXT>() noexcept {
        return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT;
    }
#endif

// VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT == 1000572001
// VK_EXT_device_generated_commands
#if defined(VK_EXT_device_generated_commands)
    template <> constexpr VkObjectType vulkanObjectType<VkIndirectExecutionSetEXT>() noexcept {
        return VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT;
    }
#endif

    // ─── Section 5: Static Assertions ────────────────────────────────────────────
    // Each assertion evaluates vulkanObjectType<T>() as a constant expression,
    // verifying at compile time that the mapping is both constexpr-capable and
    // numerically correct per the Vulkan specification.
    //
    // Extension-guarded specializations are only asserted when the corresponding
    // preprocessor symbol is defined, mirroring the #if guards in Section 4.

    // ── 5.1  Core handles ─────────────────────────────────────────────────────────
    static_assert(vulkanObjectType<VkInstance>() == VK_OBJECT_TYPE_INSTANCE);
    static_assert(vulkanObjectType<VkPhysicalDevice>() == VK_OBJECT_TYPE_PHYSICAL_DEVICE);
    static_assert(vulkanObjectType<VkDevice>() == VK_OBJECT_TYPE_DEVICE);
    static_assert(vulkanObjectType<VkQueue>() == VK_OBJECT_TYPE_QUEUE);
    static_assert(vulkanObjectType<VkSemaphore>() == VK_OBJECT_TYPE_SEMAPHORE);
    static_assert(vulkanObjectType<VkCommandBuffer>() == VK_OBJECT_TYPE_COMMAND_BUFFER);
    static_assert(vulkanObjectType<VkFence>() == VK_OBJECT_TYPE_FENCE);
    static_assert(vulkanObjectType<VkDeviceMemory>() == VK_OBJECT_TYPE_DEVICE_MEMORY);
    static_assert(vulkanObjectType<VkBuffer>() == VK_OBJECT_TYPE_BUFFER);
    static_assert(vulkanObjectType<VkImage>() == VK_OBJECT_TYPE_IMAGE);
    static_assert(vulkanObjectType<VkEvent>() == VK_OBJECT_TYPE_EVENT);
    static_assert(vulkanObjectType<VkQueryPool>() == VK_OBJECT_TYPE_QUERY_POOL);
    static_assert(vulkanObjectType<VkBufferView>() == VK_OBJECT_TYPE_BUFFER_VIEW);
    static_assert(vulkanObjectType<VkImageView>() == VK_OBJECT_TYPE_IMAGE_VIEW);
    static_assert(vulkanObjectType<VkShaderModule>() == VK_OBJECT_TYPE_SHADER_MODULE);
    static_assert(vulkanObjectType<VkPipelineCache>() == VK_OBJECT_TYPE_PIPELINE_CACHE);
    static_assert(vulkanObjectType<VkPipelineLayout>() == VK_OBJECT_TYPE_PIPELINE_LAYOUT);
    static_assert(vulkanObjectType<VkRenderPass>() == VK_OBJECT_TYPE_RENDER_PASS);
    static_assert(vulkanObjectType<VkPipeline>() == VK_OBJECT_TYPE_PIPELINE);
    static_assert(vulkanObjectType<VkDescriptorSetLayout>() == VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);
    static_assert(vulkanObjectType<VkSampler>() == VK_OBJECT_TYPE_SAMPLER);
    static_assert(vulkanObjectType<VkDescriptorPool>() == VK_OBJECT_TYPE_DESCRIPTOR_POOL);
    static_assert(vulkanObjectType<VkDescriptorSet>() == VK_OBJECT_TYPE_DESCRIPTOR_SET);
    static_assert(vulkanObjectType<VkFramebuffer>() == VK_OBJECT_TYPE_FRAMEBUFFER);
    static_assert(vulkanObjectType<VkCommandPool>() == VK_OBJECT_TYPE_COMMAND_POOL);

    // ── 5.2  Vulkan 1.1 promoted handles ─────────────────────────────────────────
    static_assert(vulkanObjectType<VkSamplerYcbcrConversion>() == VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION);
    static_assert(vulkanObjectType<VkDescriptorUpdateTemplate>() == VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE);

    // ── 5.3  Vulkan 1.3 promoted handles ─────────────────────────────────────────
    static_assert(vulkanObjectType<VkPrivateDataSlot>() == VK_OBJECT_TYPE_PRIVATE_DATA_SLOT);

    // ── 5.4  KHR extension handles ────────────────────────────────────────────────
    static_assert(vulkanObjectType<VkSwapchainKHR>() == VK_OBJECT_TYPE_SWAPCHAIN_KHR);
    static_assert(vulkanObjectType<VkDisplayKHR>() == VK_OBJECT_TYPE_DISPLAY_KHR);
    static_assert(vulkanObjectType<VkDisplayModeKHR>() == VK_OBJECT_TYPE_DISPLAY_MODE_KHR);

#if defined(VK_KHR_video_queue)
    static_assert(vulkanObjectType<VkVideoSessionKHR>() == VK_OBJECT_TYPE_VIDEO_SESSION_KHR);
    static_assert(vulkanObjectType<VkVideoSessionParametersKHR>() == VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR);
#endif

#if defined(VK_KHR_deferred_host_operations)
    static_assert(vulkanObjectType<VkDeferredOperationKHR>() == VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR);
#endif

#if defined(VK_KHR_acceleration_structure)
    static_assert(vulkanObjectType<VkAccelerationStructureKHR>() == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR);
#endif

// ── 5.5  EXT / vendor extension handles ──────────────────────────────────────
#if defined(VK_EXT_debug_report)
    static_assert(vulkanObjectType<VkDebugReportCallbackEXT>() == VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT);
#endif

#if defined(VK_EXT_debug_utils)
    static_assert(vulkanObjectType<VkDebugUtilsMessengerEXT>() == VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT);
#endif

#if defined(VK_EXT_validation_cache)
    static_assert(vulkanObjectType<VkValidationCacheEXT>() == VK_OBJECT_TYPE_VALIDATION_CACHE_EXT);
#endif

#if defined(VK_NV_ray_tracing)
    static_assert(vulkanObjectType<VkAccelerationStructureNV>() == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV);
#endif

#if defined(VK_INTEL_performance_query)
    static_assert(vulkanObjectType<VkPerformanceConfigurationINTEL>() == VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL);
#endif

#if defined(VK_NV_device_generated_commands)
    static_assert(vulkanObjectType<VkIndirectCommandsLayoutNV>() == VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV);
#endif

#if defined(VK_FUCHSIA_buffer_collection)
    static_assert(vulkanObjectType<VkBufferCollectionFUCHSIA>() == VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA);
#endif

#if defined(VK_EXT_opacity_micromap)
    static_assert(vulkanObjectType<VkMicromapEXT>() == VK_OBJECT_TYPE_MICROMAP_EXT);
#endif

#if defined(VK_NV_optical_flow)
    static_assert(vulkanObjectType<VkOpticalFlowSessionNV>() == VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV);
#endif

#if defined(VK_EXT_shader_object)
    static_assert(vulkanObjectType<VkShaderEXT>() == VK_OBJECT_TYPE_SHADER_EXT);
#endif

#if defined(VK_KHR_pipeline_binary)
    static_assert(vulkanObjectType<VkPipelineBinaryKHR>() == VK_OBJECT_TYPE_PIPELINE_BINARY_KHR);
#endif

#if defined(VK_EXT_device_generated_commands)
    static_assert(vulkanObjectType<VkIndirectCommandsLayoutEXT>() == VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT);
    static_assert(vulkanObjectType<VkIndirectExecutionSetEXT>() == VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT);
#endif

}  // namespace vkutil