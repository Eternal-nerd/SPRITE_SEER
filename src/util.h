#pragma once

#include "../lib/stb_image.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

#include <optional>
#include <string>
#include <vector>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <chrono>

#include <iostream>
#include <set>
#include <sstream>
#include <fstream>

// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
// TYPES and GLOBALS
// debug vs release global variables
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// misc. global variables
const int FPS_MEASURE_INTERVAL = 500;
const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 800;
const int MAX_FRAMES_IN_FLIGHT = 2;
const int MAX_QUADS = 2048;
const int MAX_LINES = 256;
const float PLAYER_ACCELERATION = 1.f; 
const float PLAYER_DECELERATION = 3.f;
const float PLAYER_GRAVITY = 50.f;
const float PLAYER_JUMP_VELOCITY = 3.f;
const float MAX_PLAYER_VELOCITY = 3.f;


// GAME VARIABLES

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME };

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

// vertex data structure
struct Vertex {
    glm::vec2 pos;
    glm::vec2 texCoord;
    int texIndex;
    int interaction;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[2].offset = offsetof(Vertex, texIndex);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[3].offset = offsetof(Vertex, interaction);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && texCoord == other.texCoord && texIndex == other.texIndex && other.interaction == other.interaction;
    }
};

// Used for Vulkan device selection
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// for filtering sprites to be shown
typedef enum GameScreens {
    MENU = 0,
    GAMEPLAY = 1,
    END = 2,
    NONE = 3,
} GameScreens;

struct KeyState {
    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;
    bool space = false;
    bool shift = false;
    bool ctrl = false;
};

// state variables for the whole program
struct GameState {
    bool initialized = false;

    GameScreens currentScreen;
    VkExtent2D extent;
    float spriteScale = 1.f;

    // mouse stuff
    bool mouseDown = false;
    glm::vec2 mousePos = { 0.f,0.f };
    glm::vec2 oldMousePos = { 0.f,0.f };

    // keys
    KeyState keys{};

    // time
    std::chrono::time_point<std::chrono::high_resolution_clock> programStartTime;
    float currentSimulationTime = 0.f;
    float simulationTimeDelta = 0.f;

    bool needTriangleRemap = true;
    bool needLineRemap = true;

    int wireframeTextureIndex = -1;
};

// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
// General utility
void log(const std::string& src, const std::string& msg);

// Vulkan utility
// debug
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
// physical device selection
bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions);

// SWAPCHAIN details
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, SDL_Window* window);
// Depth
VkFormat findDepthFormat(const VkPhysicalDevice& physicalDevice);

// Image shit
VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkDevice& device);
void createImage(uint32_t width, uint32_t height, VkFormat format,	VkImageTiling tiling, VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkDevice& device, VkPhysicalDevice& physicalDevice);
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkDevice device, 
    VkCommandPool commandPool, VkQueue graphicsQueue);
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkDevice device, VkCommandPool commandPool, 
    VkQueue graphicsQueue);

// BUffers/memory stuff
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, const VkPhysicalDevice& physicalDevice);
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, 
    VkDeviceMemory& bufferMemory, const VkDevice& device, const VkPhysicalDevice& physicalDevice);
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkPhysicalDevice physicalDevice, VkDevice device, 
    VkCommandPool commandPool, VkQueue graphicsQueue);

// COMMAND BUFFERS
VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);

// SHADERS
VkShaderModule createShaderModule(const std::string& filename, const VkDevice& device);
std::vector<char> readFile(const std::string& filename);
