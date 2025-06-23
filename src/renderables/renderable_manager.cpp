#include "renderable_manager.h"

/*
-----~~~~~=====<<<<<{_INITIALIZATION_}>>>>>=====~~~~~-----
*/
void RenderableManager::init(GameState& gameState, AssetManager& assetManager) {
	log(name_ + __func__, "initializing renderable manager");

	gameState_ = &gameState;
	assetManager_ = &assetManager;

	generateWorld();
}

void RenderableManager::generateWorld() {
	log(name_ + __func__, "generating game world");

	// sky
	Rectangle sky;
	sky.create(*gameState_, GAMEPLAY, false, "sky", { -1.f, -1.f }, { 1.f, 1.f }, assetManager_->getTextureIndex("../res/img/png/sky2.png"));
	rectangles_.push_back(sky);

	// floor
	Rectangle floor;
	floor.create(*gameState_, GAMEPLAY, true, "floor", { -1.f, 0.75f }, { 1.f, 0.125f }, assetManager_->getTextureIndex("../res/img/png/floor.png"));
	rectangles_.push_back(floor);


	// last = player
	player_.init(*gameState_, { 0,0 }, { 0.02f, 0.1f }, assetManager_->getTextureIndex("../res/img/png/player.png"));
}

/*
-----~~~~~=====<<<<<{_UPDATES_}>>>>>=====~~~~~-----
*/
void RenderableManager::updateAll() {
	// for now, just update player
	player_.update();
}

int RenderableManager::mapAll(Vertex* mapped) {
	//log(name_ + __func__, "Mapping everything to the buffer");

	int offset = 0;
	int vertexCount = 0;

	// rectangles
	for (int i = 0; i < rectangles_.size(); i++) {
		offset = rectangles_[i].map(mapped);
		mapped += offset;
		vertexCount += offset;
	}

	// other


	// LAST = player
	offset = player_.map(mapped);
	mapped += offset;
	vertexCount += offset;

	return vertexCount;
}

void RenderableManager::scale() {
	// rectangles
	for (int i = 0; i < rectangles_.size(); i++) {
		rectangles_[i].scale();
	}

	// other

	player_.scale();

	// re-map
	gameState_->needTriangleRemap = true;
}

void RenderableManager::onKey() {
	// for now, just update player
	player_.onKey();

	// re-map
	gameState_->needTriangleRemap = true;
}

/*
-----~~~~~=====<<<<<{_CLEANUP_}>>>>>=====~~~~~-----
*/
void RenderableManager::cleanup() {
	// rectangles
	for (int i = 0; i < rectangles_.size(); i++) {
		rectangles_[i].destroy();
	}

	// other

}
