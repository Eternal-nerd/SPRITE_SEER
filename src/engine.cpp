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
    
    // start clock
    state_.programStartTime = std::chrono::high_resolution_clock::now();

    // init vulkan/SDL
    initSDL();
    initVulkan();

    // init gamestate
    state_.currentScreen = MENU;
    state_.extent = swapChainExtent_;
    state_.initialized = true;
    state_.spriteScale = 1.f;
    state_.wireframeTextureIndex = assetManager_.getTextureIndex("../res/img/png/green.png");

    // init renderables
    renderableManager_.init(state_, assetManager_);

    // init simulation time delta
    auto simStartTime = std::chrono::high_resolution_clock::now();
    state_.currentSimulationTime = std::chrono::duration<float, std::chrono::seconds::period>(simStartTime - state_.programStartTime).count();
    state_.simulationTimeDelta = 0.f;

}

// executes repeatedly until a stop event is detected
void Engine::mainLoop() {
    log(name_ + __func__, "executing engine main loop");

    running_ = true;
    while (running_) {
        auto startTime = std::chrono::high_resolution_clock::now();
	    float frameStart = std::chrono::duration<float, std::chrono::seconds::period>(startTime - state_.programStartTime).count();
 
        handleEvents();
        waitForFrame();
        stepSimulation();
        updateBuffers();

        if (visible_) {
            // render stuff
            renderWorld();
        }
        
        // FIXME slow it down
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //std::cout << "------------FRAME-----------------\n";
        auto endTime = std::chrono::high_resolution_clock::now();
	    float frameEnd = std::chrono::duration<float, std::chrono::seconds::period>(endTime - state_.programStartTime).count();

        fpsTime_ += frameEnd - frameStart;
        loopsMeasured_++;

        if (loopsMeasured_ > FPS_MEASURE_INTERVAL) {
            float fps = 1 / (fpsTime_ / FPS_MEASURE_INTERVAL);
            log(name_ + __func__, "FPS: " + std::to_string(fps));
            fpsTime_ = 0.f;
            loopsMeasured_ = 0;
        }
    
    }

    vkDeviceWaitIdle(device_);
}

void Engine::cleanup() {
    log(name_ + __func__, "cleaning up engine");

    // swapchain
    cleanupVkSwapchain();

    log(name_ + __func__, "cleaning up renderable manager");
    renderableManager_.cleanup();

    log(name_ + __func__, "cleaning up asset manager");
    assetManager_.cleanup();

    // buffers
    // vertex buffer
    log(name_ + __func__, "destroying vertex buffer");
    vkDestroyBuffer(device_, vertexBuffer_, nullptr);
    vkFreeMemory(device_, vertexBufferMemory_, nullptr);

    //index
    log(name_ + __func__, "destroying index buffer");
    vkDestroyBuffer(device_, indexBuffer_, nullptr);
    vkFreeMemory(device_, indexBufferMemory_, nullptr);

    // line vertex
    log(name_ + __func__, "destroying line vertex buffer");
    vkDestroyBuffer(device_, lineVertexBuffer_, nullptr);
    vkFreeMemory(device_, lineVertexBufferMemory_, nullptr);

    // pipeline 
    log(name_ + __func__, "destroying graphics pipeline");
    vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
    log(name_ + __func__, "destroying pipeline layout");
    vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
    log(name_ + __func__, "destroying pipeline cache");
    vkDestroyPipelineCache(device_, pipelineCache_, nullptr);

    // render pass
    log(name_ + __func__, "destroying render pass");
    vkDestroyRenderPass(device_, renderPass_, nullptr);

    // UBO
    /*log(name_ + __func__, "destroying uniform buffers");
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device_, uniformBuffers_[i], nullptr);
        vkFreeMemory(device_, uniformBuffersMemory_[i], nullptr);
    }*/

    // descriptor stuff
    log(name_ + __func__, "destroying descriptor pool");
    vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);

    log(name_ + __func__, "destroying descriptor set layout");
    vkDestroyDescriptorSetLayout(device_, descriptorSetLayout_, nullptr);

    // snyc stuff
    log(name_ + __func__, "destroying semaphores and fences");
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device_, renderFinishedSemaphores_[i], nullptr);
        vkDestroySemaphore(device_, imageAvailableSemaphores_[i], nullptr);
        vkDestroyFence(device_, inFlightFences_[i], nullptr);
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

    // loading external stuff -----------------------------==================<
    vkCmdSetPolygonModeEXT = reinterpret_cast<PFN_vkCmdSetPolygonModeEXT>(vkGetDeviceProcAddr(device_, "vkCmdSetPolygonModeEXT"));
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
    log(name_ + __func__, "creating descriptor stuff");
    // Vertex buffer --------------------------------------------=========<
    log(name_ + __func__, "creating vertex buffer");
    VkDeviceSize vertexBufferSize = MAX_QUADS * sizeof(Vertex) * 4;
    createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer_, vertexBufferMemory_, device_, physicalDevice_);

    // Index buffer --------------------------------------------=========<
    log(name_ + __func__, "creating index buffer");
    VkDeviceSize indexBufferSize = MAX_QUADS * sizeof(uint32_t) * 6;
    createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexBuffer_, indexBufferMemory_, device_, physicalDevice_);

    // LINE buffers 
    // vertex
    log(name_ + __func__, "creating line buffer");
    VkDeviceSize lineVertexBufferSize = MAX_LINES * sizeof(Vertex) * 2;
    createBuffer(lineVertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        lineVertexBuffer_, lineVertexBufferMemory_, device_, physicalDevice_);

    // Descriptor ------------------------------------------=============<
    log(name_ + __func__, "creating descriptor pool");
    int textureCount = assetManager_.getTextureCount();
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = textureCount;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(device_, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    // Descriptor set layout -------------------------------=========<
    log(name_ + __func__, "creating descriptor set layout");
    std::array<VkDescriptorSetLayoutBinding, 1> setLayoutBindings;
    setLayoutBindings[0].binding = 0;
    setLayoutBindings[0].descriptorCount = textureCount;
    setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[0].pImmutableSamplers = nullptr;
    setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    layoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    // Descriptor set ---------------------------------------=============<
    log(name_ + __func__, "creating descriptor set");
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout_;

    if (vkAllocateDescriptorSets(device_, &allocInfo, &descriptorSet_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // Descriptors for the font images -------------------------=====<
    std::vector<VkDescriptorImageInfo> textureDescriptors(textureCount);
    for (int i = 0; i < textureCount; i++) {
        textureDescriptors[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textureDescriptors[i].imageView = assetManager_.getTextureImageView(i);
        textureDescriptors[i].sampler = assetManager_.getTextureImageSampler(i);
    }

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet_;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].descriptorCount = static_cast<uint32_t>(textureCount);
    descriptorWrites[0].pImageInfo = textureDescriptors.data();

    vkUpdateDescriptorSets(device_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Engine::createVkGraphicsPipeline() {
    log(name_ + __func__, "creating graphics pipeline");

    VkShaderModule vertShaderModule = createShaderModule("../shaders/compiled/main_vert.spv", device_);
    VkShaderModule fragShaderModule = createShaderModule("../shaders/compiled/main_frag.spv", device_);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.push_back(vertShaderStageInfo);
    shaderStages.push_back(fragShaderStageInfo);

    // Pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    if (vkCreatePipelineCache(device_, &pipelineCacheCreateInfo, nullptr, &pipelineCache_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline cache!");
    }

    // Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;

    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // Enable blending, using alpha from red channel of the font texture (see text.frag)
    VkPipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.blendEnable = VK_TRUE;
    blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // IMPORTANT!!! VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; //
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT; // VK_CULL_MODE_NONE; // 
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &blendAttachmentState;

    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_FALSE;
    depthStencilState.depthWriteEnable = VK_FALSE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;


    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VK_FALSE; // remove?
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
        VK_DYNAMIC_STATE_POLYGON_MODE_EXT
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicState.pDynamicStates = dynamicStateEnables.data();


    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = pipelineLayout_;
    pipelineCreateInfo.renderPass = renderPass_;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    if (vkCreateGraphicsPipelines(device_, pipelineCache_, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device_, fragShaderModule, nullptr);
    vkDestroyShaderModule(device_, vertShaderModule, nullptr);
}

void Engine::createVkSyncObjects() {
    log(name_ + __func__, "creating vulkan sync objects");
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS ||
            vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Engine::createVkUniformBuffers() {
    log(name_ + __func__, "creating vulkan uniform buffers ( TODO MAYBE NOT NEEDED )");
    log(name_ + __func__, "FYI: these are not being created this function is EMPTY");
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
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            handleKeyEvent();
            break;
        }
    }
}

void Engine::handleKeyEvent() {
    // KEY INPUT
    bool down = event_.type == SDL_EVENT_KEY_DOWN;
    switch (event_.key.scancode) {
    case SDL_SCANCODE_W:
        state_.keys.w = down;
        break;
    case SDL_SCANCODE_A:
        state_.keys.a = down;
        break;
    case SDL_SCANCODE_S:
        state_.keys.s = down;
        break;
    case SDL_SCANCODE_D:
        state_.keys.d = down;
        break;
    case SDL_SCANCODE_SPACE:
        state_.keys.space = down;
        break;
    case SDL_SCANCODE_LSHIFT:
        state_.keys.shift = down;
        break;
    case SDL_SCANCODE_LCTRL:
        state_.keys.ctrl = down;
        break;
    default: break;
    }

    // now do stuff?
    renderableManager_.onKey();
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
    // update game state
    state_.extent = swapChainExtent_;
    renderableManager_.scale();
}

void Engine::waitForFrame() {
    // wait for frame to be ready before mapping buffers?
    vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(device_, swapChain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame_], VK_NULL_HANDLE, &imageIndex_);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateVkSwapchain();
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
}

void Engine::stepSimulation() {
    // update simulation time delta
    auto newCurrentSimulationTime = std::chrono::high_resolution_clock::now();
    // first, update the delta using the old time
    float currentTime = std::chrono::duration<float, std::chrono::seconds::period>(newCurrentSimulationTime - state_.programStartTime).count();
    state_.simulationTimeDelta = currentTime - state_.currentSimulationTime;
    // update the current sim time
    state_.currentSimulationTime = currentTime;
    renderableManager_.updateAll();
}

void Engine::updateBuffers() {
    // update buffers here ------------------------<<<<<<<<<<<<<<<<
    if (state_.needTriangleRemap) {
        indexCount_ = 0;
        int vertexCount = 0;

        // triangle buffer
        if (vkMapMemory(device_, vertexBufferMemory_, 0, VK_WHOLE_SIZE, 0, (void**)&vertexMapped_) != VK_SUCCESS) {
            throw std::runtime_error("failed to map vertex buffer memory for overlay update ");
        }

        assert(vertexMapped_ != nullptr);

        vertexCount = renderableManager_.mapAll(vertexMapped_);

        // points should be divisible by 4 no remainder
        if (vertexCount % 4 != 0) {
            throw std::runtime_error("game pointCount not divisible by 4, pointCount % 4 = " + std::to_string(vertexCount % 4));
        }

        // vertex buffer
        vkUnmapMemory(device_, vertexBufferMemory_);
        vertexMapped_ = nullptr;

        // INDEX MAPPING ---------------------------------------<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

        // populate index buffer
        if (vkMapMemory(device_, indexBufferMemory_, 0, VK_WHOLE_SIZE, 0, (void**)&indexMapped_) != VK_SUCCESS) {
            throw std::runtime_error("failed to map index buffer memory for overlay update ");
        }

        assert(indexMapped_ != nullptr);

        std::vector<int> indices = { 0,1,2,2,1,3 };
        for (int i = 0; i < vertexCount / 4; i++) {
            for (int j = 0; j < indices.size(); j++) {
                *indexMapped_ = (indices[j] + (4 * i));
                indexMapped_++;
                indexCount_++;
            }
        }

        vkUnmapMemory(device_, indexBufferMemory_);
        indexMapped_ = nullptr;

        // reset state
        state_.needTriangleRemap = false;
    }
}

void Engine::renderWorld() {
    VkCommandBuffer commandBuffer = setupVkCommandBuffer();

    // draw buffers
    drawCalls(commandBuffer);

    submitVkCommandBuffer(commandBuffer);
}

VkCommandBuffer Engine::setupVkCommandBuffer() {
    // create next command buffer
    VkCommandBuffer commandBuffer = commandBuffers_[currentFrame_];

    vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = swapChainFramebuffers_[imageIndex_];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent_;

    // TODO fuck with this
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent_.width;
    viewport.height = (float)swapChainExtent_.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent_;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Set polygon mode and line width
    vkCmdSetPolygonModeEXT(commandBuffer, currentPolygonMode_);

    return commandBuffer;
}

void Engine::drawCalls(VkCommandBuffer commandBuffer) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);

    // Set polygon mode and line width
    vkCmdSetPolygonModeEXT(commandBuffer, currentPolygonMode_);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &descriptorSet_, 0, NULL);

    VkDeviceSize offsets = 0;

    // DRAW TRIANGLES
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer_, &offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexCount_), 1, 0, 0, 0);

    // DRAW LINES
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &lineVertexBuffer_, &offsets);
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(linePointCount_), 1, 0, 0);
}

void Engine::submitVkCommandBuffer(VkCommandBuffer commandBuffer) {
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores_[currentFrame_] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores_[currentFrame_] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFences_[currentFrame_]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // PRESENT ----------------------------------------======================<
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain_ };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex_;

    VkResult result = vkQueuePresentKHR(presentQueue_, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateVkSwapchain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
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
