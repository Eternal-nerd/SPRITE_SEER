#include "asset_manager.h"

void AssetManager::init(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue) {
    log(name_ + __func__, "initializing asset manager");

    physicalDevice_ = physicalDevice;
    device_ = device;
    commandPool_ = commandPool;
    graphicsQueue_ = graphicsQueue;

}


void AssetManager::cleanup() {}
