#pragma once

#include <vulkan/vulkan.h>

#include <string>

#include "util.h"

class Texture {
public:
	void create();

	//const VkImageView& getTextureImageView() const;
	//const VkSampler& getTextureSampler() const;
	int getIndex();

	void destroy();

	const std::string name_ = "texture ";

private:
	// name of picture file
	const char* filename_;
	
	// index of texture in list
	int index_ = -1;

	// references to vk stuff
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice device_ = VK_NULL_HANDLE;
	VkCommandPool commandPool_ = VK_NULL_HANDLE;
	VkQueue graphicsQueue_ = VK_NULL_HANDLE;

	// TEXTURE STUFF
	VkImage textureImage_ = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory_ = VK_NULL_HANDLE;
	VkImageView textureImageView_ = VK_NULL_HANDLE;
	VkSampler textureSampler_ = VK_NULL_HANDLE;
	VkImageLayout imageLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;

};