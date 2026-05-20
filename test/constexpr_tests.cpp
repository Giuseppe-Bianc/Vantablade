// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-container-size-empty, *-move-const-arg, *-move-const-arg, *-pass-by-value, *-diagnostic-self-assign-overloaded, *-unused-using-decls, *-identifier-length, *-pro-bounds-constant-array-index, *-owning-memory, cert-err33-c, *-avoid-c-arrays, *-unsafe-functions, *-pro-bounds-array-to-pointer-decay, *-use-concise-preprocessor-directives, *-const-correctness)
// clang-format on
#include <catch2/catch_test_macros.hpp>

#include <Vantablade/Device.hpp>
#include <Vantablade/vantablade.hpp>
#include <string_view>

TEST_CASE("Vulkan constexpr mappings remain stable", "[vulkan][constexpr]") {
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_DEVICE)} == "DEVICE");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_COMMAND_POOL)} == "COMMAND_POOL");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_MICROMAP_EXT)} == "MICROMAP_EXT");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_PRIVATE_DATA_SLOT)} == "PRIVATE_DATA_SLOT");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_SWAPCHAIN_KHR)} == "SWAPCHAIN_KHR");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_TENSOR_ARM)} == "TENSOR_ARM");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_TENSOR_VIEW_ARM)} == "TENSOR_VIEW_ARM");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_PIPELINE_BINARY_KHR)} == "PIPELINE_BINARY_KHR");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_DATA_GRAPH_PIPELINE_SESSION_ARM)} == "DATA_GRAPH_PIPELINE_SESSION_ARM");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_EXTERNAL_COMPUTE_QUEUE_NV)} == "EXTERNAL_COMPUTE_QUEUE_NV");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT)} == "INDIRECT_COMMANDS_LAYOUT_EXT");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT)} == "INDIRECT_EXECUTION_SET_EXT");

    STATIC_REQUIRE(vkutil::vulkanObjectType<VkInstance>() == VK_OBJECT_TYPE_INSTANCE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkPhysicalDevice>() == VK_OBJECT_TYPE_PHYSICAL_DEVICE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDevice>() == VK_OBJECT_TYPE_DEVICE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkQueue>() == VK_OBJECT_TYPE_QUEUE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkSemaphore>() == VK_OBJECT_TYPE_SEMAPHORE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkFence>() == VK_OBJECT_TYPE_FENCE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDeviceMemory>() == VK_OBJECT_TYPE_DEVICE_MEMORY);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkBuffer>() == VK_OBJECT_TYPE_BUFFER);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkImage>() == VK_OBJECT_TYPE_IMAGE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkImageView>() == VK_OBJECT_TYPE_IMAGE_VIEW);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkPipelineLayout>() == VK_OBJECT_TYPE_PIPELINE_LAYOUT);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkCommandPool>() == VK_OBJECT_TYPE_COMMAND_POOL);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDescriptorSetLayout>() == VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDescriptorPool>() == VK_OBJECT_TYPE_DESCRIPTOR_POOL);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDescriptorSet>() == VK_OBJECT_TYPE_DESCRIPTOR_SET);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkFramebuffer>() == VK_OBJECT_TYPE_FRAMEBUFFER);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkSurfaceKHR>() == VK_OBJECT_TYPE_SURFACE_KHR);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkSamplerYcbcrConversion>() == VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDescriptorUpdateTemplate>() == VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkSwapchainKHR>() == VK_OBJECT_TYPE_SWAPCHAIN_KHR);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDisplayKHR>() == VK_OBJECT_TYPE_DISPLAY_KHR);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDisplayModeKHR>() == VK_OBJECT_TYPE_DISPLAY_MODE_KHR);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkPrivateDataSlot>() == VK_OBJECT_TYPE_PRIVATE_DATA_SLOT);

#if defined(VK_KHR_video_queue)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkVideoSessionKHR>() == VK_OBJECT_TYPE_VIDEO_SESSION_KHR);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkVideoSessionParametersKHR>() == VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR);
#endif
#if defined(VK_KHR_deferred_host_operations)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDeferredOperationKHR>() == VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR);
#endif
#if defined(VK_KHR_acceleration_structure)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkAccelerationStructureKHR>() == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR);
#endif
#if defined(VK_EXT_debug_report)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDebugReportCallbackEXT>() == VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT);
#endif
#if defined(VK_EXT_debug_utils)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDebugUtilsMessengerEXT>() == VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT);
#endif
#if defined(VK_EXT_validation_cache)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkValidationCacheEXT>() == VK_OBJECT_TYPE_VALIDATION_CACHE_EXT);
#endif
#if defined(VK_NV_ray_tracing)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkAccelerationStructureNV>() == VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV);
#endif
#if defined(VK_INTEL_performance_query)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkPerformanceConfigurationINTEL>() == VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL);
#endif
#if defined(VK_NV_device_generated_commands)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkIndirectCommandsLayoutNV>() == VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV);
#endif
#if defined(VK_FUCHSIA_buffer_collection)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkBufferCollectionFUCHSIA>() == VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA);
#endif
#if defined(VK_EXT_opacity_micromap)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkMicromapEXT>() == VK_OBJECT_TYPE_MICROMAP_EXT);
#endif
#if defined(VK_NV_optical_flow)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkOpticalFlowSessionNV>() == VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV);
#endif
#if defined(VK_EXT_shader_object)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkShaderEXT>() == VK_OBJECT_TYPE_SHADER_EXT);
#endif
#if defined(VK_KHR_pipeline_binary)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkPipelineBinaryKHR>() == VK_OBJECT_TYPE_PIPELINE_BINARY_KHR);
#endif
#if defined(VK_EXT_device_generated_commands)
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkIndirectCommandsLayoutEXT>() == VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkIndirectExecutionSetEXT>() == VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT);
#endif

    STATIC_REQUIRE(DebugColors::Red[0] == 1.0F);
    STATIC_REQUIRE(DebugColors::Red[1] == 0.0F);
    STATIC_REQUIRE(DebugColors::Green[1] == 1.0F);
    STATIC_REQUIRE(DebugColors::Blue[2] == 1.0F);
    STATIC_REQUIRE(DebugColors::None[3] == 0.0F);
}

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-container-size-empty, *-move-const-arg, *-move-const-arg, *-pass-by-value, *-diagnostic-self-assign-overloaded, *-unused-using-decls, *-identifier-length, *-pro-bounds-constant-array-index, *-owning-memory, cert-err33-c, *-avoid-c-arrays, *-unsafe-functions, *-pro-bounds-array-to-pointer-decay, *-use-concise-preprocessor-directives, *-const-correctness)
// clang-format on