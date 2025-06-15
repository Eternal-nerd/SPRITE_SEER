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
    // decelerate if no input and there is still velocity
    // TODO FIXME
    if (acceleration_.x == 0.f && velocity_.x != 0.f) {
        //acceleration_.x = (velocity_.x < 0.f) ? -PLAYER_ACCELERATION : PLAYER_ACCELERATION;
    }

    // FIXME just STOPPING velocity if no input
    if (acceleration_ == glm::vec2(0.f,0.f)) {
        velocity_ = {0.f,0.f};
    }

    // limit velocity
    if (velocity_.x > MAX_PLAYER_VELOCITY || velocity_.x < -MAX_PLAYER_VELOCITY) {
		velocity_.x = (velocity_.x < 0.f) ? -MAX_PLAYER_VELOCITY : MAX_PLAYER_VELOCITY;
    }
    if (velocity_.y > MAX_PLAYER_VELOCITY || velocity_.y < -MAX_PLAYER_VELOCITY) {
		velocity_.y = (velocity_.y < 0.f) ? -MAX_PLAYER_VELOCITY : MAX_PLAYER_VELOCITY;
    }

    // calculate velocity
    velocity_ = {velocity_.x + acceleration_.x * gameState_->simulationTimeDelta, velocity_.y + acceleration_.y * gameState_->simulationTimeDelta};;

	// update position based on velocity
	position_ += glm::vec2(velocity_.x, velocity_.y);

    // edge of screen collision x
    if (position_.x <= -1.f || position_.x >= (1.f - (sizePercent_.x * 2) * gameState_->spriteScale)) { 
        // reset velocity and acceleration
        velocity_.x = 0.f;
        acceleration_.x = 0.f;
       
        // limit position
        if (position_.x < 0.f) {
            position_.x = -1.f;
        }
        else {
            position_.x = 1.f - (sizePercent_.x * 2) * gameState_->spriteScale;
        }
    }

    // edge of screen collision y
    if (position_.y <= -1.f || position_.y >= (1.f - (sizePercent_.y * 2) * gameState_->spriteScale)) { 
        // reset velocity and acceleration
        velocity_.y = 0.f;
        acceleration_.y = 0.f;
       
        // limit position
        if (position_.y < 0.f) {
            position_.y = -1.f;
        }
        else {
            position_.y = 1.f - (sizePercent_.y * 2) * gameState_->spriteScale;
        }
    }

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
		acceleration_.y -= gameState_->simulationTimeDelta * PLAYER_ACCELERATION;
	}
	if (gameState_->keys.s && !gameState_->keys.w) {
		acceleration_.y += gameState_->simulationTimeDelta * PLAYER_ACCELERATION;
	}
	if ((gameState_->keys.s && gameState_->keys.w) || (!gameState_->keys.s && !gameState_->keys.w)) {
		acceleration_.y = 0;
	}
	if (gameState_->keys.d && !gameState_->keys.a) {
		acceleration_.x += gameState_->simulationTimeDelta * PLAYER_ACCELERATION;
	}
	if (gameState_->keys.a && !gameState_->keys.d) {
		acceleration_.x -= gameState_->simulationTimeDelta * PLAYER_ACCELERATION;
	}
	if ((gameState_->keys.a && gameState_->keys.d) || (!gameState_->keys.a && !gameState_->keys.d)) {
		acceleration_.x = 0;
	}
}

/*
-----~~~~~=====<<<<<{_CLEANUP_}>>>>>=====~~~~~-----
*/
void Player::cleanup() {

}
