#pragma once

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <optional>
#include <vector>
#include <array>

// debug vs release global variables
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// misc. global variables
const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 800;
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME };

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

// Used for Vulkan device selection
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
