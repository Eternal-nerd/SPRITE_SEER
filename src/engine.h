#pragma once

// third-party library includes
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "../lib/stb_image.h"

// standard library includes
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <stdexcept>

// includes from project
#include "types.h"
#include "util.h"
#include "asset_manager.h"


// main class for the whole program
class Engine {
public:
    void run();

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
    void createVkTextures();

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
    std::vector<VkCommandBuffer> commandBuffers_;
    uint32_t currentFrame_ = 0;
    

};
