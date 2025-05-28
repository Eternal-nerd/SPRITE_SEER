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
#include <sstream>

// includes from project
#include "types.h"

// debug vs release global variables
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// misc. global variables
const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 800;
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME };

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

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

    // Main loop sub-functions
    void handleEvents();

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

    // -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----

    // state variables
    bool running_ = false;
    bool visible_ = true;

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

    

};
