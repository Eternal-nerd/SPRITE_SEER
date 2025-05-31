#pragma once

#include <vulkan/vulkan.h>

#include <string>

#include "util.h"

class Texture {
public:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
	void create(const std::string& filename, VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);

	const VkImageView& getImageView() const;
	const VkSampler& getSampler() const;

	void destroy();

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	const std::string name_ = "Texture::";

private:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
	//

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	// name of picture file
	const char* filename_;

	// references to vk stuff
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice device_ = VK_NULL_HANDLE;
	VkCommandPool commandPool_ = VK_NULL_HANDLE;
	VkQueue graphicsQueue_ = VK_NULL_HANDLE;

	// TEXTURE STUFF
	VkImage image_ = VK_NULL_HANDLE;
	VkDeviceMemory imageMemory_ = VK_NULL_HANDLE;
	VkImageView imageView_ = VK_NULL_HANDLE;
	VkSampler sampler_ = VK_NULL_HANDLE;
};