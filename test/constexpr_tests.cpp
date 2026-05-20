// NOLINTBEGIN(*-include-cleaner)
#include <catch2/catch_test_macros.hpp>

#include <Vantablade/vantablade.hpp>
#include <string_view>
// NOLINTEND(*-include-cleaner)

TEST_CASE("Vulkan constexpr mappings remain stable", "[vulkan][constexpr]") {
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_DEVICE)} == "DEVICE");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_COMMAND_POOL)} == "COMMAND_POOL");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_PRIVATE_DATA_SLOT)} == "PRIVATE_DATA_SLOT");
    STATIC_REQUIRE(std::string_view{VkObjectString(VK_OBJECT_TYPE_SWAPCHAIN_KHR)} == "SWAPCHAIN_KHR");
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkInstance>() == VK_OBJECT_TYPE_INSTANCE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDevice>() == VK_OBJECT_TYPE_DEVICE);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkBuffer>() == VK_OBJECT_TYPE_BUFFER);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkCommandPool>() == VK_OBJECT_TYPE_COMMAND_POOL);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkDescriptorSetLayout>() == VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkSamplerYcbcrConversion>() == VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkSwapchainKHR>() == VK_OBJECT_TYPE_SWAPCHAIN_KHR);
    STATIC_REQUIRE(vkutil::vulkanObjectType<VkPrivateDataSlot>() == VK_OBJECT_TYPE_PRIVATE_DATA_SLOT);
}
