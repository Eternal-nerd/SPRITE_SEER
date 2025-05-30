#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "util.h"
#include "texture.h"

class AssetManager {
public:

	void init(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);

    void cleanup();

	const std::string name_ = "asset manager ";

private:
	// vk access
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice device_ = VK_NULL_HANDLE;
	VkCommandPool commandPool_ = VK_NULL_HANDLE;
	VkQueue graphicsQueue_ = VK_NULL_HANDLE;

	// Textures
	std::vector<std::string> textureFilenames_{};
	std::vector<std::string> textureNames_{};
	std::vector<Texture> textures_{};

};
