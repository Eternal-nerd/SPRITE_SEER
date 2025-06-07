#pragma once

#include "../util.h"

class Rectangle {
public:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
	void create(GameState& gameState, GameScreens screen, bool collidable, const std::string& id, glm::vec2 position, glm::vec2 sizePercent, int textureIndex);

	int map(Vertex* mapped);

	// utility
	//bool isHovered();
	void scale();

	void destroy();

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	std::string id_ = "";

	const std::string name_ = "Rectangle::";

private:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	std::array<Vertex, 4> vertices_{};

	GameScreens screen_ = NONE;
	bool collidable_ = false;

	GameState* gameState_ = nullptr;

	int textureIndex_ = -1;
	glm::vec2 position_ = { 0.f, 0.f };
	glm::vec2 sizePercent_ = { 0.f, 0.f };

};