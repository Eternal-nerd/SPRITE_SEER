#include "texture.h"

/*
-----~~~~~=====<<<<<{_INITIALIZATION_}>>>>>=====~~~~~-----
*/
void Texture::create(const std::string& filename, VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue) {
	filename_ = filename.c_str();

	physicalDevice_ = physicalDevice;
	device_ = device;
	commandPool_ = commandPool;
	graphicsQueue_ = graphicsQueue;
	
	// TEXTURE IMAGE ------------------------------====<
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename_, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("stbi_load() call failed: " + std::string(stbi_failure_reason()));
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory, device_, physicalDevice_);

	void* data;
	vkMapMemory(device_, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device_, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, imageMemory_,
		device_, physicalDevice_);

	transitionImageLayout(image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device_, commandPool_, graphicsQueue_);
	copyBufferToImage(stagingBuffer, image_, static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight), device_, commandPool_, graphicsQueue_);
	transitionImageLayout(image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device_, commandPool_, graphicsQueue_);

	vkDestroyBuffer(device_, stagingBuffer, nullptr);
	vkFreeMemory(device_, stagingBufferMemory, nullptr);

	// TEXTURE IMAGE VIEW ------------------------------====<
	imageView_ = createImageView(image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, device_);

	// TEXTURE IMAGE SAMPLER ------------------------------====<
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice_, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // FIXME -> 1.0f
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(device_, &samplerInfo, nullptr, &sampler_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

/*
-----~~~~~=====<<<<<{_GETTERS_}>>>>>=====~~~~~-----
*/
const VkImageView& Texture::getImageView() const { return imageView_; }
const VkSampler& Texture::getSampler() const { return sampler_; }

/*
-----~~~~~=====<<<<<{_CLEANUP_}>>>>>=====~~~~~-----
*/
void Texture::destroy() {
	log(name_ + __func__, "destroying vulkan texture objects");

	// CLEANUP TEXTURE stuff
	vkDestroySampler(device_, sampler_, nullptr);
	vkDestroyImageView(device_, imageView_, nullptr);
	vkDestroyImage(device_, image_, nullptr);
	vkFreeMemory(device_, imageMemory_, nullptr);
}