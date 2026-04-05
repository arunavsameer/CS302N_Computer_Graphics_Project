#include "../include/chicken.h"

Chicken::Chicken() { 
    position = glm::vec3(0.0f, Config::CELL_SIZE * 0.6f, 0.0f); 
    isJumping = false;
    jumpProgress = 0.0f;
    rotationY = 180.0f; 
}

void Chicken::update(float deltaTime) {
    if (!isJumping) return;

    jumpProgress += deltaTime / JUMP_DURATION;

    if (jumpProgress >= 1.0f) {
        position = targetPos;
        isJumping = false;
        jumpProgress = 0.0f;
    } else {
        position.x = startPos.x + (targetPos.x - startPos.x) * jumpProgress;
        position.z = startPos.z + (targetPos.z - startPos.z) * jumpProgress;
        
        float parabola = 4.0f * JUMP_HEIGHT * jumpProgress * (1.0f - jumpProgress);
        position.y = startPos.y + parabola;
    }
}

void Chicken::render(Renderer& renderer) {
    renderer.drawTexturedCube(position, glm::vec3(Config::CELL_SIZE * 0.8f), "chicken", rotationY);
}

void Chicken::move(float gridX, float gridZ) {
    if (isJumping) return; 
    if (gridX == 0.0f && gridZ == 0.0f) return;

    isJumping = true;
    startPos = position;
    targetPos = position + glm::vec3(gridX * Config::CELL_SIZE, 0.0f, gridZ * Config::CELL_SIZE);
    jumpProgress = 0.0f;

    if (gridX > 0) {
        rotationY = -90.0f; 
    } else if (gridX < 0) {
        rotationY = 90.0f;  
    } else if (gridZ < 0) {
        rotationY = 180.0f; 
    } else if (gridZ > 0) {
        rotationY = 0.0f;   
    }
}

glm::vec3 Chicken::getBasePosition() const {
    if (isJumping) {
        float baseY = startPos.y + (targetPos.y - startPos.y) * jumpProgress;
        return glm::vec3(position.x, baseY, position.z);
    }
    return position;
}

void Chicken::applyLogVelocity(float velocityX, float deltaTime) {
    if (!isJumping) {
        position.x += velocityX * deltaTime;
        startPos.x = position.x; 
        targetPos.x = position.x;
    }
}