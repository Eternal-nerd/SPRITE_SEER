#include "player.h"

/*
-----~~~~~=====<<<<<{_INITIALIZATION_}>>>>>=====~~~~~-----
*/
void Player::init(GameState& gameState, glm::vec2 position, glm::vec2 sizePercent, int textureIndex) {
	log(name_ + __func__, "init player");
	
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
    // decceleration
	if (noX_ && !stopped_) {
        if (velocity_.x < 0.f) {
            // check if stop
            if (velocity_.x + (PLAYER_DECELERATION * gameState_->simulationTimeDelta) >= 0) {
                stopped_ = true;
            }
            else {
                 acceleration_.x = PLAYER_DECELERATION;
            }
        }
        if (velocity_.x > 0.f) {
            // check if stop
            if (velocity_.x - (PLAYER_DECELERATION * gameState_->simulationTimeDelta) <= 0) {
                stopped_ = true;
            }
            else {
                 acceleration_.x = -PLAYER_DECELERATION;
            }
        }
    }
    
    // on x stop 
    if (stopped_) {
        acceleration_.x = 0.f;
        velocity_.x = 0.f;
    }
        
	// add gravity
	acceleration_.y += (airborne_) ? PLAYER_GRAVITY * gameState_->simulationTimeDelta : 0.f;

    // calculate velocity
    velocity_ += glm::vec2(acceleration_.x * gameState_->simulationTimeDelta, acceleration_.y * gameState_->simulationTimeDelta);

    // limit velocity
    if (velocity_.y > MAX_PLAYER_VELOCITY) {
		velocity_.y = MAX_PLAYER_VELOCITY;
    }
	if (velocity_.y < -MAX_PLAYER_VELOCITY) {
		velocity_.y = -MAX_PLAYER_VELOCITY;
	}

   	if (velocity_.x > MAX_PLAYER_VELOCITY) {
		velocity_.x = MAX_PLAYER_VELOCITY;
    }
	if (velocity_.x < -MAX_PLAYER_VELOCITY) {
		velocity_.x = -MAX_PLAYER_VELOCITY;
	}

	// update position based on velocity
	position_ += glm::vec2(velocity_.x * gameState_->simulationTimeDelta, velocity_.y * gameState_->simulationTimeDelta);

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

        // limit position
        if (position_.y < 0.f) {
            position_.y = -1.f;
		}
        else {
            position_.y = 1.f - (sizePercent_.y * 2) * gameState_->spriteScale;
			acceleration_.y = 0.f;
			airborne_ = false;
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
	if ((gameState_->keys.w || gameState_->keys.space) && !gameState_->keys.s && !airborne_) {
		velocity_.y = -PLAYER_JUMP_VELOCITY;
		noY_ = false;
		airborne_ = true;
	}
	if (gameState_->keys.s && !gameState_->keys.w) {
		//acceleration_.y = PLAYER_ACCELERATION;
		noY_ = false;
	}
	if ((gameState_->keys.s && gameState_->keys.w) || (!gameState_->keys.s && !gameState_->keys.w)) {
		noY_ = true;
	}
	if (gameState_->keys.d && !gameState_->keys.a) {
		acceleration_.x = PLAYER_ACCELERATION;
		noX_ = false;
        stopped_ = false;
	}
	if (gameState_->keys.a && !gameState_->keys.d) {
		acceleration_.x = -PLAYER_ACCELERATION;
		noX_ = false;
        stopped_ = false;
	}
	if (!gameState_->keys.a && !gameState_->keys.d) {
		noX_ = true;
	}
    if (gameState_->keys.a && gameState_->keys.d) {
        stopped_ = true;
    }
}

/*
-----~~~~~=====<<<<<{_CLEANUP_}>>>>>=====~~~~~-----
*/
void Player::cleanup() {

}
