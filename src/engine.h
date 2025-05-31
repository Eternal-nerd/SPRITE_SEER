#pragma once

// third-party library includes
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

// standard library includes
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <stdexcept>

// includes from project
#include "util.h"
#include "asset_manager.h"

// main class for the whole program
class Engine {
public:
    void run();

    const std::string name_ = "Engine::";
private:
    // -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
    // main 3 steps of program
    void init();
    void mainLoop();
    void cleanup();

    // init sub-functions
    void initSDL();
    void initVulkan();

    // vulkan init sub-functions
    void createVkDevice();
    void createVkCommandBuffers();
    void createVkRenderPass();
	void createVkSwapchain();
	void createVkDescriptors();
	void createVkGraphicsPipeline();
	void createVkSyncObjects();
	void createVkUniformBuffers();

	// swapchain helpers
	void recreateVkSwapchain();
	void cleanupVkSwapchain();

    // Main loop sub-functions
    void handleEvents();

    // -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
    // state variables
    bool running_ = false;
    bool visible_ = true;

    // asset manager
    AssetManager assetManager_;

    // SDL objects
    SDL_Window* windowPtr_ = nullptr;
    SDL_Event event_;

    // Vulkan objects
    // Vulkan DEVICE Stuff ------------------------==<
	VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
	VkInstance instance_ = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice device_ = VK_NULL_HANDLE;
	VkSurfaceKHR surface_ = VK_NULL_HANDLE;
	VkQueue graphicsQueue_ = VK_NULL_HANDLE;
	VkQueue presentQueue_ = VK_NULL_HANDLE;

    // Vulkan command buffers --------------------===<
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers_{};
    uint32_t currentFrame_ = 0;

	// Vulkan Renderpass ------------------------==<
	VkRenderPass renderPass_ = VK_NULL_HANDLE;

	// Vulkan Swapchain -----------------------===<
	VkSwapchainKHR swapChain_ = VK_NULL_HANDLE;
	std::vector<VkImage> swapChainImages_{};
	VkFormat swapChainImageFormat_ = VK_FORMAT_UNDEFINED;
	VkExtent2D swapChainExtent_{};
	std::vector<VkImageView> swapChainImageViews_{};
	std::vector<VkFramebuffer> swapChainFramebuffers_{};
	VkImage depthImage_ = VK_NULL_HANDLE;
	VkDeviceMemory depthImageMemory_ = VK_NULL_HANDLE;
	VkImageView depthImageView_ = VK_NULL_HANDLE;
	uint32_t imageIndex_ = 0;

	// Vulkan descriptor layout & pipeline --------------------===<
	VkDescriptorSetLayout descriptorSetLayout_ = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;

	// Vulkan synchronization ------------------------===<
	std::vector<VkSemaphore> imageAvailableSemaphores_{};
	std::vector<VkSemaphore> renderFinishedSemaphores_{};
	std::vector<VkFence> inFlightFences_{};

	// UBO ----------------------------------------===<
	std::vector<VkBuffer> uniformBuffers_{};
	std::vector<VkDeviceMemory> uniformBuffersMemory_{};
	std::vector<void*> uniformBuffersMapped_{};

	// Descriptor pool/sets ---------------------------======<
	VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorSets_{};

	// EXTERNAL Vulkan API function ptrs
	PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT{ VK_NULL_HANDLE };
};
