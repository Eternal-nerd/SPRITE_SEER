#pragma once

#include <vulkan/vulkan.h>

#include <iostream>
#include <set>
#include <sstream>

#include "types.h"

// General utility
void log(const std::string& src, const std::string& msg);


// Vulkan utility
// debug
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
// physical device selection
bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions);
