#include "../include/chicken.h"
#include "../include/types.h"

Chicken::Chicken() { 
    position = glm::vec3(0.0f, CELL_SIZE * 0.6f, 0.0f); 
    isJumping = false;
    jumpProgress = 0.0f;
    rotationY = 180.0f; // Default facing away from the camera (forward)
}

void Chicken::update(float deltaTime) {
    if (!isJumping) return;

    // Advance the jump timer
    jumpProgress += deltaTime / JUMP_DURATION;

    if (jumpProgress >= 1.0f) {
        // Snap to target at the end of the jump to prevent floating point drift
        position = targetPos;
        isJumping = false;
        jumpProgress = 0.0f;
    } else {
        // 1. Linear interpolation (Lerp) for X and Z axis
        position.x = startPos.x + (targetPos.x - startPos.x) * jumpProgress;
        position.z = startPos.z + (targetPos.z - startPos.z) * jumpProgress;
        
        // 2. Parabolic motion for Y axis
        // Equation: 4 * max_height * t * (1 - t)
        float parabola = 4.0f * JUMP_HEIGHT * jumpProgress * (1.0f - jumpProgress);
        position.y = startPos.y + parabola;
    }
}

void Chicken::render(Renderer& renderer) {
    // Pass the current rotation to the renderer
    renderer.drawTexturedCube(position, glm::vec3(CELL_SIZE * 0.8f), "chicken", rotationY);
}

void Chicken::move(float gridX, float gridZ) {
    // Ignore input if the chicken is already mid-air
    if (isJumping) return; 
    if (gridX == 0.0f && gridZ == 0.0f) return;

    // Setup jump state
    isJumping = true;
    startPos = position;
    targetPos = position + glm::vec3(gridX * CELL_SIZE, 0.0f, gridZ * CELL_SIZE);
    jumpProgress = 0.0f;

    // Set rotation based on movement direction
    if (gridX > 0) {
        rotationY = -90.0f; // Facing Right
    } else if (gridX < 0) {
        rotationY = 90.0f;  // Facing Left
    } else if (gridZ < 0) {
        rotationY = 180.0f; // Facing Forward (away from camera)
    } else if (gridZ > 0) {
        rotationY = 0.0f;   // Facing Backward (towards camera)
    }
}

// NEW: Return the position without the parabolic jump arc
glm::vec3 Chicken::getBasePosition() const {
    if (isJumping) {
        // Linearly interpolate Y in case of platform elevation changes, ignoring the jump parabola
        float baseY = startPos.y + (targetPos.y - startPos.y) * jumpProgress;
        return glm::vec3(position.x, baseY, position.z);
    }
    return position;
}