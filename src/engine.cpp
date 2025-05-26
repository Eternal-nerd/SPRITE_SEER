#include "engine.h"

void Engine::run() {
    std::cout << "running engine\n";

    init();
    mainLoop();
    cleanup();
}

void Engine::init() {
    std::cout << "initializing engine\n";
    
    // init vulkan/SDL
    createDevice();
}

void Engine::mainLoop() {
    std::cout << "executing engine main loop\n";

    
    
}

void Engine::cleanup() {
    std::cout << "cleaning up engine\n";
}

void Engine::createDevice() {
    std::cout << "creating vulkan device\n";

}
