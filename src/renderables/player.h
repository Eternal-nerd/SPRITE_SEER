#pragma once

#include "../util.h"

class Player {
public:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
	void init(GameState& gameState, glm::vec2 position, glm::vec2 sizePercent, int textureIndex);

	void update();

	int map(Vertex* mapped);

	// utility
	void scale();
	void onKey();

	void cleanup();

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	const std::string name_ = "Player::";

private:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	std::array<Vertex, 4> vertices_{};

	GameState* gameState_ = nullptr;

	int textureIndex_ = -1;
	glm::vec2 velocity_ = { 0.f, 0.f };
	glm::vec2 position_ = { 0.f, 0.f };
	glm::vec2 sizePercent_ = { 0.f, 0.f };

};