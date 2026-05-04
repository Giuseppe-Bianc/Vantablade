/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "headers.hpp"

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