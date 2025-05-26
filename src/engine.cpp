#include "engine.h"

/*
-----~~~~~=====<<<<<{_ONLY_PUBLIC_METHOD_}>>>>>=====~~~~~-----
*/
void Engine::run() {
    std::cout << "running engine\n";

    init();
    mainLoop();
    cleanup();
}

/*
-----~~~~~=====<<<<<{_MAIN_CONTROL_FLOW_METHODS_}>>>>>=====~~~~~-----
*/
void Engine::init() {
    std::cout << "initializing engine\n";
    
    // init vulkan/SDL
    initSDL();
    initVulkan();


}

void Engine::mainLoop() {
    std::cout << "executing engine main loop\n";

    running_ = true;
    while (running_) {

        handleEvents();

        if (visible_) {
            // render stuff
        }
        
    }
    
}

void Engine::cleanup() {
    std::cout << "cleaning up engine\n";
}

/*
-----~~~~~=====<<<<<{_SUB_INITIALIZATION_METHODS_}>>>>>=====~~~~~-----
*/
void Engine::initSDL() {
    std::cout << "initializing SDL\n";

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        throw std::runtime_error("failed to initialize SDL");
    }

    // TODO ADD COOL ICON
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
}

void Engine::initVulkan() {
    std::cout << "initializing Vulkan\n";

    createVkDevice();

}

void Engine::createVkDevice() {
    std::cout << "creating Vulkan device\n";

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

        // populate debug stuff
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;

        instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = nullptr;
    }

    // INSTANCE CREATE CALL HERE
    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    /*
    // Vulkan debug layer --------------------====<
    // setup debug messager if in debug mode
    if (enableValidationLayers) {
        util::log(name_, "setting up debug messenger");
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        util::populateDebugMessengerCreateInfo(createInfo);

        if (util::CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    // Vulkan/SDL surface --------------------====<
    util::log(name_, "creating surface");
    if (!SDL_Vulkan_CreateSurface(window_, instance_, nullptr, &surface_)) {
        throw std::runtime_error("failed to create SDL window surface!");
    }

    // TODO: print device selected to logger!!!!
    // Vulkan physical device (GPU) --------------------====<
    util::log(name_, "selecting physical device");
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (util::isDeviceSuitable(device, surface_, deviceExtensions)) {
            physicalDevice_ = device;
            break;
        }
    }

    if (physicalDevice_ == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    // Vulkan LOGICAL Device --------------------====<
    util::log(name_, "creating logical device");
    QueueFamilyIndices indices = util::findQueueFamilies(physicalDevice_, surface_);

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

    util::log(name_, "checking vulkan device features");
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

    if (vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    util::log(name_, "getting device queues");
    vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
    */
}


/*
-----~~~~~=====<<<<<{_VULKAN_HELPER_METHODS_}>>>>>=====~~~~~-----
*/
VKAPI_ATTR VkBool32 VKAPI_CALL Engine::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "[Vulkan validation layer]: " << pCallbackData->pMessage << "\n";
    return VK_FALSE;
}

/*
-----~~~~~=====<<<<<{_SUB_MAIN_LOOP_METHODS_}>>>>>=====~~~~~-----
*/
void Engine::handleEvents() {
    // poll for events
    while (SDL_PollEvent(&event_)) {
        switch (event_.type) {
        case SDL_EVENT_QUIT:
            std::cout << "quit event happened\n";
            running_ = false;
            break;
        }
    }
}
