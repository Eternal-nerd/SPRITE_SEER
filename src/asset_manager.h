#pragma once

#include <vulkan/vulkan.h>

#include "texture.h"


class AssetManager {
public:

	//void init(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);

private:
	// vk access
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice device_ = VK_NULL_HANDLE;
	VkCommandPool commandPool_ = VK_NULL_HANDLE;
	VkQueue graphicsQueue_ = VK_NULL_HANDLE;


};
