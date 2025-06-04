#pragma once

#include "util.h"

class Rectangle {
public:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----
	void create(OverlayState& state, OverlayElementState* elementState, const std::string& id, glm::vec2 position, glm::vec2 sizeUnits, const std::string& textureName);

	int map(Vertex* mapped);

	void 

	void destroy();

	// utility
	bool isHovered();

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	std::string id_ = "";

	const std::string name_ = "Rectangle::";

private:
	// -----~~~~~=====<<<<<{_METHODS_}>>>>>=====~~~~~-----

	// -----~~~~~=====<<<<<{_VARIABLES_}>>>>>=====~~~~~-----
	std::array<Vertex, 4> vertices{};

	bool unique_ = false;

	GameState* gameState_ = nullptr;
	SpriteState* spriteState_ = nullptr;

	glm::vec2 position_ = { 0.f, 0.f };
	// units -> pixels defined in utils.h
	glm::vec2 sizeUnits_ = { 0.f, 0.f };

};