#include "engine.h"

/*
-----~~~~~=====<<<<<{_ONLY_PUBLIC_METHOD_}>>>>>=====~~~~~-----
*/
void Engine::run() {
    log(__func__, "running engine");

    init();
    mainLoop();
    cleanup();
}

/*
-----~~~~~=====<<<<<{_MAIN_CONTROL_FLOW_METHODS_}>>>>>=====~~~~~-----
*/
// run once on startup, initializes the program
void Engine::init() {
    log(__func__, "initializing engine");
    
    // init vulkan/SDL
    initSDL();
    initVulkan();


}

// executes repeatedly until a stop event is detected
void Engine::mainLoop() {
    log(__func__, "executing engine main loop");

    running_ = true;
    while (running_) {

        handleEvents();

        if (visible_) {
            // render stuff
        }
        
    }
    
}

void Engine::cleanup() {
    log(__func__, "cleaning up engine");

    log(__func__, "destroying command pool");
    vkDestroyCommandPool(device_, commandPool_, nullptr);

    // Devices/instance
    log(__func__, "destroying logical device");
    vkDestroyDevice(device_, nullptr);

    log(__func__, "destroying surface");
    vkDestroySurfaceKHR(instance_, surface_, nullptr);

    if (enableValidationLayers) {
        log(__func__, "destroying debug messenger");
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance_, debugMessenger_, nullptr);
        }
    }

    log(__func__, "destroying Vulkan instance");
    vkDestroyInstance(instance_, nullptr);

    // SDL
    log(__func__, "cleaning up SDL");
    SDL_DestroyWindow(windowPtr_);
    SDL_Quit();
}

/*
-----~~~~~=====<<<<<{_SUB_INITIALIZATION_METHODS_}>>>>>=====~~~~~-----
*/
void Engine::initSDL() {
    log(__func__, "initializing SDL");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        throw std::runtime_error("failed to initialize SDL");
    }

    log(__func__, "creating SDL window");
    windowPtr_ = SDL_CreateWindow("SPRITE_SEER", WIDTH, HEIGHT, SDL_WINDOW_VULKAN);
    if (!windowPtr_) {
        throw std::runtime_error("failed to create SDL window");
    }

    if (!SDL_SetWindowResizable(windowPtr_, true)) {
        throw std::runtime_error("failed to make window resizable");
    }
    
    /*
    if (!SDL_SetWindowRelativeMouseMode(windowPtr_, true)) {
        throw std::runtime_error("failed to put mouse into relative mode");
    }
    */

    // ADD COOL ICON FOR WINDOW
    int imgWidth, imgHeight, imgChannels;
    stbi_uc* pixels = stbi_load("../res/icon/icon.jpg", &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);

    if (!pixels) {
        std::cout << "unable to load icon image: -> " << stbi_failure_reason() << "\n";
        throw std::runtime_error("failed to load window icon image");
    }


    SDL_Surface* surfaceIcon = nullptr;
    surfaceIcon = SDL_CreateSurfaceFrom(imgWidth, imgHeight, SDL_PIXELFORMAT_RGBA32, pixels, imgWidth*4);

    if (!surfaceIcon) {
        throw std::runtime_error("failed to create cool icon SDL surface");
    }

    log(__func__, "setting SDL window icon");
    if (!SDL_SetWindowIcon(windowPtr_, surfaceIcon)) {
        throw std::runtime_error("failed to set cool window icon");
    }

    stbi_image_free(pixels);
    SDL_DestroySurface(surfaceIcon);
}

void Engine::initVulkan() {
    log(__func__, "initializing Vulkan");

    createVkDevice();
    createVkCommandBuffers();
    createVkTextures();

}

void Engine::createVkDevice() {
    log(__func__, "creating Vulkan device");

    // Vulkan instance --------------------====<
    // validation layer check
    bool validationLayersSupported = false;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                validationLayersSupported = true;
                break;
            }
        }
    }

    if (enableValidationLayers && !validationLayersSupported) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "SPRITE_SEER";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    // get extensions (SDL):
    std::vector<const char*> extensions;
    uint32_t sdl_extension_count = 0;
    const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count);
    for (uint32_t i = 0; i < sdl_extension_count; i++) {
        extensions.push_back(sdl_extensions[i]);
    }

    if (enableValidationLayers) {
        // need to add debug util to extensions
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    // if using validation layer, we need to set the debug callback messenger stuff
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = nullptr;
    }

    // INSTANCE CREATE CALL HERE
    log(__func__, "creating vulkan instance");
    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    // Vulkan debug layer --------------------====<
    // setup debug messager if in debug mode
    if (enableValidationLayers) {
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
        populateDebugMessengerCreateInfo(messengerCreateInfo);

        VkResult checkResult;
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            log(__func__, "creating vulkan debug utils messenger");
            checkResult = func(instance_, &messengerCreateInfo, nullptr, &debugMessenger_);
        }
        else {
            checkResult = VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        if (checkResult != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    // Vulkan/SDL surface --------------------====<
    log(__func__, "creating SDL/Vulkan window surface");
    if (!SDL_Vulkan_CreateSurface(windowPtr_, instance_, nullptr, &surface_)) {
        throw std::runtime_error("failed to create SDL window surface!");
    }

    // TODO: print device selected to logger!!!!
    // Vulkan physical device (GPU) --------------------====<
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device, surface_, deviceExtensions)) {
            log(__func__, "selected vulkan physical device");
            physicalDevice_ = device;
            break;
        }
    }

    if (physicalDevice_ == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    // Vulkan LOGICAL Device --------------------====<
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice_, surface_);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features = {};
    dynamicState3Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;

    VkPhysicalDeviceVulkan12Features vulkan12Features = {};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12Features.pNext = &dynamicState3Features;

    VkPhysicalDeviceFeatures2 physicalFeatures2 = {};
    physicalFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalFeatures2.pNext = &vulkan12Features;

    vkGetPhysicalDeviceFeatures2(physicalDevice_, &physicalFeatures2);

    // Logic if feature is not supported
    if (vulkan12Features.runtimeDescriptorArray && vulkan12Features.shaderSampledImageArrayNonUniformIndexing && dynamicState3Features.extendedDynamicState3PolygonMode == VK_FALSE) {
        throw std::runtime_error("needed features not enabled on chosen device");
    }

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pNext = &physicalFeatures2;
    deviceCreateInfo.pEnabledFeatures = NULL;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    log(__func__, "creating vulkan logical device");
    if (vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
}

void Engine::createVkCommandBuffers() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice_, surface_);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    log(__func__, "creating vulkan command pool");
    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }

    commandBuffers_.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers_.size();

    log(__func__, "allocating vulkan command buffers");
    if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Engine::createVkTextures() {



}

/*
-----~~~~~=====<<<<<{_SUB_MAIN_LOOP_METHODS_}>>>>>=====~~~~~-----
*/
void Engine::handleEvents() {
    // poll for events
    while (SDL_PollEvent(&event_)) {
        switch (event_.type) {
        case SDL_EVENT_QUIT:
            log(__func__, "quit event happened");
            running_ = false;
            break;
        }
    }
}
