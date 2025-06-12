#include "player.h"

/*
-----~~~~~=====<<<<<{_INITIALIZATION_}>>>>>=====~~~~~-----
*/
void Player::init(GameState& gameState, glm::vec2 position, glm::vec2 sizePercent, int textureIndex) {
	gameState_ = &gameState;

	position_ = position;
	sizePercent_ = sizePercent;
	textureIndex_ = textureIndex;

	// initialize vertices
	float xOffset = (sizePercent_.x * 2) * gameState_->spriteScale;
	float yOffset = (sizePercent_.y * 2) * gameState_->spriteScale;

	// top left
	vertices_[0].pos = { position_.x, position_.y };
	vertices_[0].texCoord = { 0, 0 };
	vertices_[0].texIndex = textureIndex_;
	vertices_[0].interaction = 0;

	// bottom left
	vertices_[1].pos = { position_.x, position_.y + yOffset };
	vertices_[1].texCoord = { 0, 1 };
	vertices_[1].texIndex = textureIndex_;
	vertices_[1].interaction = 0;

	// top right
	vertices_[2].pos = { position_.x + xOffset, position_.y };
	vertices_[2].texCoord = { 1, 0 };
	vertices_[2].texIndex = textureIndex_;
	vertices_[2].interaction = 0;

	// bottom right
	vertices_[3].pos = { position_.x + xOffset, position_.y + yOffset };
	vertices_[3].texCoord = { 1, 1 };
	vertices_[3].texIndex = textureIndex_;
	vertices_[3].interaction = 0;
}

/*
-----~~~~~=====<<<<<{_UPDATES_}>>>>>=====~~~~~-----
*/
void Player::update() {
	// update position based on velocity
	position_.x += velocity_.x * gameState_->simulationTimeDelta*100;
	position_.y += velocity_.y * gameState_->simulationTimeDelta*100;

	scale();

	gameState_->needTriangleRemap = true;
}

int Player::map(Vertex* mapped) {
	for (int i = 0; i < 4; i++) {
		mapped->pos.x = vertices_[i].pos.x; // position x
		mapped->pos.y = vertices_[i].pos.y; // position y
		mapped->texCoord.x = vertices_[i].texCoord.x; // tex coord x
		mapped->texCoord.y = vertices_[i].texCoord.y; // tex coord y
		mapped->texIndex = vertices_[i].texIndex; // tex index
		mapped->interaction = vertices_[i].interaction; // for checking hover
		mapped++;
	}
	return 4;
}


void Player::scale() {
	// calculate x and y offsets
	float xOffset = (sizePercent_.x * 2) * gameState_->spriteScale;
	float yOffset = (sizePercent_.y * 2) * gameState_->spriteScale;

	// update position if needed?
	vertices_[0].pos = position_;

	// bottom left
	vertices_[1].pos = { position_.x, position_.y + yOffset };

	// top right
	vertices_[2].pos = { position_.x + xOffset, position_.y };

	// bottom right
	vertices_[3].pos = { position_.x + xOffset, position_.y + yOffset };
}

void Player::onKey() {
	if (gameState_->keys.w && !gameState_->keys.s) {
		velocity_.y -= gameState_->simulationTimeDelta;
	}
	if (gameState_->keys.s && !gameState_->keys.w) {
		velocity_.y += gameState_->simulationTimeDelta;
	}
	if ((gameState_->keys.s && gameState_->keys.w) || (!gameState_->keys.s && !gameState_->keys.w)) {
		velocity_.y = 0;
	}
	if (gameState_->keys.d && !gameState_->keys.a) {
		velocity_.x += gameState_->simulationTimeDelta;
	}
	if (gameState_->keys.a && !gameState_->keys.d) {
		velocity_.x -= gameState_->simulationTimeDelta;
	}
	if ((gameState_->keys.a && gameState_->keys.d) || (!gameState_->keys.a && !gameState_->keys.d)) {
		velocity_.x = 0;
	}

	if (velocity_.x > MAX_PLAYER_VELOCITY) {
		velocity_.x = MAX_PLAYER_VELOCITY;
	}

	if (velocity_.y > MAX_PLAYER_VELOCITY) {
		velocity_.y = MAX_PLAYER_VELOCITY;
	}
}

/*
-----~~~~~=====<<<<<{_CLEANUP_}>>>>>=====~~~~~-----
*/
void Player::cleanup() {

}