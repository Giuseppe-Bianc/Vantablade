/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "headers.hpp"

#include <vulkan/vulkan.h>

[[nodiscard]] static constexpr const char *VkObjectString(VkObjectType v) noexcept {
    switch(v) {
    case VK_OBJECT_TYPE_UNKNOWN:
        return "UNKNOWN";
    case VK_OBJECT_TYPE_INSTANCE:
        return "INSTANCE";
    case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
        return "PHYSICAL_DEVICE";
    case VK_OBJECT_TYPE_DEVICE:
        return "DEVICE";
    case VK_OBJECT_TYPE_QUEUE:
        return "QUEUE";
    case VK_OBJECT_TYPE_SEMAPHORE:
        return "SEMAPHORE";
    case VK_OBJECT_TYPE_COMMAND_BUFFER:
        return "COMMAND_BUFFER";
    case VK_OBJECT_TYPE_FENCE:
        return "FENCE";
    case VK_OBJECT_TYPE_DEVICE_MEMORY:
        return "DEVICE_MEMORY";
    case VK_OBJECT_TYPE_BUFFER:
        return "BUFFER";
    case VK_OBJECT_TYPE_IMAGE:
        return "IMAGE";
    case VK_OBJECT_TYPE_EVENT:
        return "EVENT";
    case VK_OBJECT_TYPE_QUERY_POOL:
        return "QUERY_POOL";
    case VK_OBJECT_TYPE_BUFFER_VIEW:
        return "BUFFER_VIEW";
    case VK_OBJECT_TYPE_IMAGE_VIEW:
        return "IMAGE_VIEW";
    case VK_OBJECT_TYPE_SHADER_MODULE:
        return "SHADER_MODULE";
    case VK_OBJECT_TYPE_PIPELINE_CACHE:
        return "PIPELINE_CACHE";
    case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
        return "PIPELINE_LAYOUT";
    case VK_OBJECT_TYPE_RENDER_PASS:
        return "RENDER_PASS";
    case VK_OBJECT_TYPE_PIPELINE:
        return "PIPELINE";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
        return "DESCRIPTOR_SET_LAYOUT";
    case VK_OBJECT_TYPE_SAMPLER:
        return "SAMPLER";
    case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
        return "DESCRIPTOR_POOL";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET:
        return "DESCRIPTOR_SET";
    case VK_OBJECT_TYPE_FRAMEBUFFER:
        return "FRAMEBUFFER";
    case VK_OBJECT_TYPE_COMMAND_POOL:
        return "COMMAND_POOL";
    case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
        return "DESCRIPTOR_UPDATE_TEMPLATE";
    case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
        return "SAMPLER_YCBCR_CONVERSION";
    case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT:
        return "PRIVATE_DATA_SLOT";
    case VK_OBJECT_TYPE_SURFACE_KHR:
        return "SURFACE_KHR";
    case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
        return "SWAPCHAIN_KHR";
    case VK_OBJECT_TYPE_DISPLAY_KHR:
        return "DISPLAY_KHR";
    case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
        return "DISPLAY_MODE_KHR";
    case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
        return "DEBUG_REPORT_CALLBACK_EXT";
    case VK_OBJECT_TYPE_VIDEO_SESSION_KHR:
        return "VIDEO_SESSION_KHR";
    case VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR:
        return "VIDEO_SESSION_PARAMETERS_KHR";
    case VK_OBJECT_TYPE_CU_MODULE_NVX:
        return "CU_MODULE_NVX";
    case VK_OBJECT_TYPE_CU_FUNCTION_NVX:
        return "CU_FUNCTION_NVX";
    case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
        return "DEBUG_UTILS_MESSENGER_EXT";
    case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:
        return "ACCELERATION_STRUCTURE_KHR";
    case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
        return "VALIDATION_CACHE_EXT";
    case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
        return "ACCELERATION_STRUCTURE_NV";
    case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL:
        return "PERFORMANCE_CONFIGURATION_INTEL";
    case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR:
        return "DEFERRED_OPERATION_KHR";
    case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:
        return "INDIRECT_COMMANDS_LAYOUT_NV";
#ifdef VK_ENABLE_BETA_EXTENSIONS
    case VK_OBJECT_TYPE_CUDA_MODULE_NV:
        return "CUDA_MODULE_NV";
    case VK_OBJECT_TYPE_CUDA_FUNCTION_NV:
        return "CUDA_FUNCTION_NV";
#endif
    case VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA:
        return "BUFFER_COLLECTION_FUCHSIA";
    case VK_OBJECT_TYPE_MICROMAP_EXT:
        return "MICROMAP_EXT";
    case VK_OBJECT_TYPE_TENSOR_ARM:
        return "TENSOR_ARM";
    case VK_OBJECT_TYPE_TENSOR_VIEW_ARM:
        return "TENSOR_VIEW_ARM";
    case VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV:
        return "OPTICAL_FLOW_SESSION_NV";
    case VK_OBJECT_TYPE_SHADER_EXT:
        return "SHADER_EXT";
    case VK_OBJECT_TYPE_PIPELINE_BINARY_KHR:
        return "PIPELINE_BINARY_KHR";
    case VK_OBJECT_TYPE_DATA_GRAPH_PIPELINE_SESSION_ARM:
        return "DATA_GRAPH_PIPELINE_SESSION_ARM";
    case VK_OBJECT_TYPE_EXTERNAL_COMPUTE_QUEUE_NV:
        return "EXTERNAL_COMPUTE_QUEUE_NV";
    case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT:
        return "INDIRECT_COMMANDS_LAYOUT_EXT";
    case VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT:
        return "INDIRECT_EXECUTION_SET_EXT";
    default:
        return "UNHANDLED";
    }
}

[[nodiscard]] static inline const char *VkDebugUtilsMessageTypeFlagBitsEXTString(VkDebugUtilsMessageTypeFlagBitsEXT messageType) noexcept {
    switch(messageType) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        return "[GENERAL] ";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        return "[VALIDATION] ";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        return "[PERFORMANCE] ";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
        return "[DEVICE_ADDRESS_BINDING] ";
    default:
        return "Unhandled VkDebugUtilsMessageTypeFlagBitsEXT";
    }
}

static inline const char *VkMemoryPropertyFlagBitsString(VkMemoryPropertyFlagBits input_value) noexcept {
    switch(input_value) {
    case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT:
        return "DEVICE_LOCAL";
    case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT:
        return "HOST_VISIBLE";
    case VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
        return "HOST_COHERENT";
    case VK_MEMORY_PROPERTY_HOST_CACHED_BIT:
        return "HOST_CACHED";
    case VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT:
        return "LAZILY_ALLOCATED";
    case VK_MEMORY_PROPERTY_PROTECTED_BIT:
        return "PROTECTED";
    case VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD:
        return "DEVICE_COHERENT";
    case VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD:
        return "DEVICE_UNCACHED";
    case VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV:
        return "RDMA_CAPABLE";
    default:
        return "Unhandled VkMemoryPropertyFlagBits";
    }
}

static inline const char *VkQueueFlagBitsString(VkQueueFlagBits input_value) noexcept {
    switch(input_value) {
    case VK_QUEUE_GRAPHICS_BIT:
        return "GRAPHICS";
    case VK_QUEUE_COMPUTE_BIT:
        return "COMPUTE";
    case VK_QUEUE_TRANSFER_BIT:
        return "TRANSFER";
    case VK_QUEUE_SPARSE_BINDING_BIT:
        return "SPARSE_BINDING";
    case VK_QUEUE_PROTECTED_BIT:
        return "PROTECTED";
    case VK_QUEUE_VIDEO_DECODE_BIT_KHR:
        return "VIDEO_DECODE";
    case VK_QUEUE_VIDEO_ENCODE_BIT_KHR:
        return "VIDEO_ENCODE";
    case VK_QUEUE_OPTICAL_FLOW_BIT_NV:
        return "OPTICAL_FLOW";
    default:
        return "Unhandled VkQueueFlagBits";
    }
}

namespace detail {
    // TMPL: concept-constrained generic implementation eliminates O(n) duplication
    template <std::unsigned_integral FlagType, typename BitType, typename BitToStringFn>
    [[nodiscard]] std::string flags_to_string_impl(FlagType input_value, BitToStringFn bit_to_string, std::string_view separator) {
        // SAFETY: preserve original zero-input behavior (empty string)
        if(input_value == 0) return {};

        std::string ret;
        // PERF: modest baseline reserve; geometric growth handles outliers without over-allocating
        ret.reserve(64);

        bool first = true;
        auto remaining = input_value;  // mutated per loop iteration

        while(remaining != 0) {
            // PERF: std::countr_zero jumps directly to the next set bit using TZCNT/BSF.
            // Complexity drops from O(32) to O(k) where k = number of set bits.
            const auto shift = std::countr_zero(remaining);
            const auto bit = static_cast<FlagType>(1) << shift;

            if(!first) { ret.append(separator); }
            // TMPL: invokes the Vulkan-specific bit-to-string helper
            ret.append(bit_to_string(static_cast<BitType>(bit)));
            first = false;

            // Clear the processed bit to advance the scan
            remaining &= ~bit;
        }
        return ret;
    }
}  // namespace detail

// PUBLIC INTERFACES PRESERVED EXACTLY
[[nodiscard]] static inline std::string VkMemoryPropertyFlagsString(VkMemoryPropertyFlags input_value) {
    return detail::flags_to_string_impl<VkMemoryPropertyFlags, VkMemoryPropertyFlagBits>(
        input_value, [](VkMemoryPropertyFlagBits b) { return VkMemoryPropertyFlagBitsString(b); },
        " | ");  // Standardized separator for readability
}

[[nodiscard]] static inline std::string VkQueueFlagsString(VkQueueFlags input_value) {
    return detail::flags_to_string_impl<VkQueueFlags, VkQueueFlagBits>(
        input_value, [](VkQueueFlagBits b) { return VkQueueFlagBitsString(b); }, " | ");
}

[[nodiscard]] static inline std::string VkDebugUtilsMessageTypeFlagsEXTString(VkDebugUtilsMessageTypeFlagsEXT input_value) {
    return detail::flags_to_string_impl<VkDebugUtilsMessageTypeFlagsEXT, VkDebugUtilsMessageTypeFlagBitsEXT>(
        input_value, [](VkDebugUtilsMessageTypeFlagBitsEXT b) { return VkDebugUtilsMessageTypeFlagBitsEXTString(b); },
        " | ");  // Unified separator; original used "|" inconsistently
}

// NOLINTEND(*-include-cleaner)