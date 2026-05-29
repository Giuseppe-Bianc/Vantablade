/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN
#pragma once

#include "headers.hpp"

// vulkan headers
#include <vulkan/vk_enum_string_helper.h>

#include "vulkanToString.hpp"

#define VK_CHECK(f, throwable)                                                                                                             \
    do {                                                                                                                                   \
        const VkResult res = (f);                                                                                                          \
        if(res != VK_SUCCESS) [[unlikely]] {                                                                                               \
            const auto loc = std::source_location::current();                                                                              \
            const std::string detailed_message = FORMAT("VkResult is \"{}\" from {} in {} at line {} in function {}",                      \
                                                        string_VkResult(res), #f, loc.file_name(), loc.line(), loc.function_name());       \
            LCRITICAL(detailed_message);                                                                                                   \
            throw std::runtime_error(FORMAT("{}: {}", throwable, detailed_message));                                                       \
        }                                                                                                                                  \
    } while(0)

#define VK_CHECK_SWAPCHAIN(f, trowable)                                                                                                    \
    do {                                                                                                                                   \
        const VkResult res = (f);                                                                                                          \
        if(res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) [[unlikely]] {                                                                   \
            constexpr auto loc = std::source_location::current();                                                                          \
            LCRITICAL("Fatal : VkResult is \"{0}\" from{1} in {2} at line {3}", #f, string_VkResult(res), loc.file_name(), loc.line());    \
            throw std::runtime_error(trowable);                                                                                            \
        }                                                                                                                                  \
    } while(0)

// NOLINTEND