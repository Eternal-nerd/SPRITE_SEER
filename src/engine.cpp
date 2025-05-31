#include "engine.h"

/*
-----~~~~~=====<<<<<{_ONLY_PUBLIC_METHOD_}>>>>>=====~~~~~-----
*/
void Engine::run() {
    log(name_ + __func__, "running engine");

    init();
    mainLoop();
    cleanup();
}

/*
-----~~~~~=====<<<<<{_MAIN_CONTROL_FLOW_METHODS_}>>>>>=====~~~~~-----
*/
// run once on startup, initializes the program
void Engine::init() {
    log(name_ + __func__, "initializing engine");
    
    // init vulkan/SDL
    initSDL();
    initVulkan();


}

// executes repeatedly until a stop event is detected
void Engine::mainLoop() {
    log(name_ + __func__, "executing engine main loop");

    running_ = true;
    while (running_) {

        handleEvents();

        if (visible_) {
            // render stuff
        }
        
    }
    
}

void Engine::cleanup() {
    log(name_ + __func__, "cleaning up engine");

    // swapchain
    cleanupVkSwapchain();

    log(name_ + __func__, "cleaning up asset manager");
    assetManager_.cleanup();

    // pipeline & layout
    log(name_ + __func__, "destroying graphics pipeline");
    vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
    log(name_ + __func__, "destroying pipeline layout");
    vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);

    // render pass
    log(name_ + __func__, "destroying render pass");
    vkDestroyRenderPass(device_, renderPass_, nullptr);

    // UBO
    log(name_ + __func__, "destroying uniform buffers");
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        //vkDestroyBuffer(device_, uniformBuffers_[i], nullptr);
        //vkFreeMemory(device_, uniformBuffersMemory_[i], nullptr);
    }

    // descriptor stuff
    log(name_ + __func__, "destroying descriptor pool");
    vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);

    log(name_ + __func__, "destroying descriptor set layout");
    vkDestroyDescriptorSetLayout(device_, descriptorSetLayout_, nullptr);

    // snyc stuff
    log(name_ + __func__, "destroying semaphores and fences");
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        //vkDestroySemaphore(device_, renderFinishedSemaphores_[i], nullptr);
        //vkDestroySemaphore(device_, imageAvailableSemaphores_[i], nullptr);
        //vkDestroyFence(device_, inFlightFences_[i], nullptr);
    }

    log(name_ + __func__, "destroying command pool");
    vkDestroyCommandPool(device_, commandPool_, nullptr);

    // Devices/instance
    log(name_ + __func__, "destroying logical device");
    vkDestroyDevice(device_, nullptr);

    log(name_ + __func__, "destroying surface");
    vkDestroySurfaceKHR(instance_, surface_, nullptr);

    if (enableValidationLayers) {
        log(name_ + __func__, "destroying debug messenger");
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance_, debugMessenger_, nullptr);
        }
    }

    log(name_ + __func__, "destroying Vulkan instance");
    vkDestroyInstance(instance_, nullptr);

    // SDL
    log(name_ + __func__, "cleaning up SDL");
    SDL_DestroyWindow(windowPtr_);
    SDL_Quit();
}

/*
-----~~~~~=====<<<<<{_SUB_INITIALIZATION_METHODS_}>>>>>=====~~~~~-----
*/
void Engine::initSDL() {
    log(name_ + __func__, "initializing SDL");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        throw std::runtime_error("failed to initialize SDL");
    }

    log(name_ + __func__, "creating SDL window");
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
        std::cout << "unable to load icon image: " << stbi_failure_reason() << "\n";
        throw std::runtime_error("failed to load window icon image");
    }


    SDL_Surface* surfaceIcon = nullptr;
    surfaceIcon = SDL_CreateSurfaceFrom(imgWidth, imgHeight, SDL_PIXELFORMAT_RGBA32, pixels, imgWidth*4);

    if (!surfaceIcon) {
        throw std::runtime_error("failed to create cool icon SDL surface");
    }

    log(name_ + __func__, "setting SDL window icon");
    if (!SDL_SetWindowIcon(windowPtr_, surfaceIcon)) {
        throw std::runtime_error("failed to set cool window icon");
    }

    //stbi_image_free(pixels);
    SDL_DestroySurface(surfaceIcon);
}

void Engine::initVulkan() {
    log(name_ + __func__, "initializing Vulkan");

    createVkDevice();
    createVkCommandBuffers();
    // creates the textures
    assetManager_.init(physicalDevice_, device_, commandPool_, graphicsQueue_);
    createVkRenderPass();
    createVkSwapchain();
    createVkDescriptors();
    createVkGraphicsPipeline();
    createVkSyncObjects();
    createVkUniformBuffers();
}

void Engine::createVkDevice() {
    log(name_ + __func__, "creating Vulkan device");

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
    log(name_ + __func__, "creating vulkan instance");
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
            log(name_ + __func__, "creating vulkan debug utils messenger");
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
    log(name_ + __func__, "creating SDL/Vulkan window surface");
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
            log(name_ + __func__, "selected vulkan physical device");
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

    log(name_ + __func__, "creating vulkan logical device");
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

    log(name_ + __func__, "creating vulkan command pool");
    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }

    commandBuffers_.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers_.size();

    log(name_ + __func__, "allocating vulkan command buffers");
    if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Engine::createVkRenderPass() {
    log(name_ + __func__, "creating renderpass");

    // get some swapchain details here:
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice_, surface_);

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = chooseSwapSurfaceFormat(swapChainSupport.formats).format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat(physicalDevice_);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {
        colorAttachment,
        depthAttachment
    };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Engine::createVkSwapchain() {
    log(name_ + __func__, "creating swapchain");
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice_, surface_);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, windowPtr_);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapCreateInfo{};
    swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapCreateInfo.surface = surface_;

    swapCreateInfo.minImageCount = imageCount;
    swapCreateInfo.imageFormat = surfaceFormat.format;
    swapCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapCreateInfo.imageExtent = extent;
    swapCreateInfo.imageArrayLayers = 1;
    swapCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice_, surface_);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        swapCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapCreateInfo.queueFamilyIndexCount = 2;
        swapCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        swapCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapCreateInfo.presentMode = presentMode;
    swapCreateInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device_, &swapCreateInfo, nullptr, &swapChain_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
    swapChainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());

    swapChainImageFormat_ = surfaceFormat.format;
    swapChainExtent_ = extent;

    // IMAGE VIEWS ----------------------------=====<
    log(name_ + __func__, "creating swapchain image views");
    swapChainImageViews_.resize(swapChainImages_.size());

    for (uint32_t i = 0; i < swapChainImages_.size(); i++) {
        swapChainImageViews_[i] = createImageView(
            swapChainImages_[i],
            swapChainImageFormat_,
            VK_IMAGE_ASPECT_COLOR_BIT,
            device_
        );
    }

    // DEPTH RESOURCES --------------------================<
    log(name_ + __func__, "creating depth resources");
    VkFormat depthFormat = findDepthFormat(physicalDevice_);

    createImage(
        swapChainExtent_.width,
        swapChainExtent_.height,
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage_,
        depthImageMemory_,
        device_,
        physicalDevice_
    );

    depthImageView_ = createImageView(
        depthImage_,
        depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        device_
    );

    // FRAMEBUFFERS -------------------------===<
    log(name_ + __func__, "creating framebuffers");
    swapChainFramebuffers_.resize(swapChainImageViews_.size());

    for (size_t i = 0; i < swapChainImageViews_.size(); i++) {
        std::array<VkImageView, 2> attachments = { swapChainImageViews_[i], depthImageView_ };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent_.width;
        framebufferInfo.height = swapChainExtent_.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &swapChainFramebuffers_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Engine::createVkDescriptors() {
    log(name_ + __func__, "creating descriptor set layout");
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = static_cast<uint32_t>(assetManager_.getTextureCount());
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    // do the rest of the descriptor stuff here

}

void Engine::createVkGraphicsPipeline() {
    log(name_ + __func__, "creating graphics pipeline");

    //VkShaderModule vertShaderModule = createShaderModule("../shaders/compiled/main_vert.spv", device_);
    //VkShaderModule fragShaderModule = createShaderModule("../shaders/compiled/main_frag.spv", device_);

}

void Engine::createVkSyncObjects() {
    log(name_ + __func__, "creating vulkan sync objects");
}

void Engine::createVkUniformBuffers() {
    log(name_ + __func__, "creating vulkan uniform buffers");
}


/*
-----~~~~~=====<<<<<{_SUB_MAIN_LOOP_METHODS_}>>>>>=====~~~~~-----
*/
void Engine::handleEvents() {
    // poll for events
    while (SDL_PollEvent(&event_)) {
        switch (event_.type) {
        case SDL_EVENT_QUIT:
            log(name_ + __func__, "quit event happened");
            running_ = false;
            break;
        }
    }
}

void Engine::recreateVkSwapchain() {
    log(name_ + __func__, "recreating swapchain");
    vkDeviceWaitIdle(device_);
    cleanupVkSwapchain();
    createVkSwapchain();
    log(name_ + __func__, "new swapchain extent: ["
        + std::to_string(swapChainExtent_.width)
        + ", "
        + std::to_string(swapChainExtent_.height)
        + "]"
    );
}

/*
-----~~~~~=====<<<<<{_SUB_CLEANUP_METHODS_}>>>>>=====~~~~~-----
*/
void Engine::cleanupVkSwapchain() {
    log(name_ + __func__, "cleaning up swapchain");

    log(name_ + __func__, "destroying swapchain depth resources");
    vkDestroyImageView(device_, depthImageView_, nullptr);
    vkDestroyImage(device_, depthImage_, nullptr);
    vkFreeMemory(device_, depthImageMemory_, nullptr);

    log(name_ + __func__, "destroying swapchain frame buffers");
    for (auto framebuffer : swapChainFramebuffers_) {
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
    }

    log(name_ + __func__, "destroying swapchain image views");
    for (auto imageView : swapChainImageViews_) {
        vkDestroyImageView(device_, imageView, nullptr);
    }

    log(name_ + __func__, "destroying swapchain");
    vkDestroySwapchainKHR(device_, swapChain_, nullptr);
}