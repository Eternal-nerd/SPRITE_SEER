#pragma once

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>

#include <vector>
#include <filesystem>

#include "util.h"
#include "texture.h"

namespace fs = std::filesystem;

class AssetManager {
public:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
	void init(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);

	// get total amount of textures
	int getTextureCount();

	// by index in textures_ vector
	const VkImageView& getTextureImageView(int index) const;
	const VkSampler& getTextureImageSampler(int index) const;

	// texture index by filename
	int getTextureIndex(const std::string& filename);

	// play audio sound
	void playSound(const std::string& filename);

    void cleanup();

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	const std::string name_ = "AssetManager::";

private:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
	void enumerateFiles();
	void initTextures();
	void initAudio();

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	// vk access
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice device_ = VK_NULL_HANDLE;
	VkCommandPool commandPool_ = VK_NULL_HANDLE;
	VkQueue graphicsQueue_ = VK_NULL_HANDLE;

	// Textures (int is the texture index)
	std::vector<std::string> textureFilenames_{};
	std::vector<Texture> textures_{};

	// Audio
	std::vector<std::string> audioFilenames_{};
	SDL_AudioStream* stream_ = NULL;
	Uint8* wavData_ = NULL;
	Uint32 wavDataLen_ = 0;
	SDL_AudioSpec audioSpec_{};

};
