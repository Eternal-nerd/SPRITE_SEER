#pragma once

#include "../util.h"
#include "../asset_manager.h"
#include "rectangle.h"
#include "player.h"


class RenderableManager {
public:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
	void init(GameState& gameState, AssetManager& assetManager);

	void updateAll();

	int mapAll(Vertex* mapped);

	void scale();
	void onKey();

	void cleanup();

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	const std::string name_ = "RenderableManager::";

private:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
	void generateWorld();

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	GameState* gameState_ = nullptr;
	AssetManager* assetManager_ = nullptr;

	Player player_;
	std::vector<Rectangle> rectangles_{};
};