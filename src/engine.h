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
#include <thread>

// includes from project
#include "util.h"
#include "asset_manager.h"
#include "renderables/renderable_manager.h"

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
    void handleEvents(); // input handling step
	void handleKeyEvent();
	void waitForFrame();
	void stepSimulation();
	void updateBuffers(); // updating buffers with new vertex data based on sim (MAYBE USE UNIFORM BUFFER INSTEAD??)
	void renderWorld(); // makes vk command buffer, draws everything, submits command buffer

	// render world sub functions
	VkCommandBuffer setupVkCommandBuffer();
	void drawCalls(VkCommandBuffer commandBuffer);
	void submitVkCommandBuffer(VkCommandBuffer commandBuffer);


    // -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
    // state variables
    bool running_ = false;
    bool visible_ = true;
    float fpsTime_ = 0.f;
	int loopsMeasured_ = 0;

	// game state
	GameState state_{};
	
    // asset manager
    AssetManager assetManager_;

	// game object manager
	RenderableManager renderableManager_;

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

	// Vulkan descriptor layout/pool/sets --------------------===<
	VkDescriptorSetLayout descriptorSetLayout_ = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
	// TODO MAYBE NEED MORE OF THESE???????
	VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;

	// BUFFERS ---------------------------======<
	// triangles
	VkBuffer vertexBuffer_ = VK_NULL_HANDLE;
	VkDeviceMemory vertexBufferMemory_ = VK_NULL_HANDLE;
	VkBuffer indexBuffer_ = VK_NULL_HANDLE;
	VkDeviceMemory indexBufferMemory_ = VK_NULL_HANDLE;
	// lines
	VkBuffer lineVertexBuffer_ = VK_NULL_HANDLE;
	VkDeviceMemory lineVertexBufferMemory_ = VK_NULL_HANDLE;

	// memory mapped vertex buffer
	Vertex* vertexMapped_ = nullptr;
	uint32_t* indexMapped_ = nullptr;
	int indexCount_ = 0;
	Vertex* lineVertexMapped_ = nullptr;
	int linePointCount_ = 0;

	// Pipeline ---------------------------======<
	VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
	VkPipelineCache pipelineCache_ = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;
	VkPolygonMode currentPolygonMode_ = VK_POLYGON_MODE_FILL;

	// Vulkan synchronization ------------------------===<
	std::vector<VkSemaphore> imageAvailableSemaphores_{};
	std::vector<VkSemaphore> renderFinishedSemaphores_{};
	std::vector<VkFence> inFlightFences_{};

	// UBO ----------------------------------------===< DONT NEED YET
	//std::vector<VkBuffer> uniformBuffers_{};
	//std::vector<VkDeviceMemory> uniformBuffersMemory_{};
	//std::vector<void*> uniformBuffersMapped_{};

	// EXTERNAL Vulkan API function ptrs
	PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT{ VK_NULL_HANDLE };
};
