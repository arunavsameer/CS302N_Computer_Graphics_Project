#ifndef CHICKEN_H
#define CHICKEN_H

#include <glm/glm.hpp>
#include "renderer.h"

class Chicken {
private:
    glm::vec3 position;
    
    // --- Jump State Variables ---
    bool isJumping;
    glm::vec3 startPos;
    glm::vec3 targetPos;
    float jumpProgress; // Normalized time from 0.0 to 1.0
    float rotationY;    // Current facing angle in degrees

public:
    // --- Hyperparameters ---
    static constexpr float JUMP_DURATION = 0.25f; // How long the jump takes (in seconds)
    static constexpr float JUMP_HEIGHT   = 1.5f;  // Peak height of the jump

    Chicken();
    
    void update(float deltaTime);
    void render(Renderer& renderer);
    void move(float gridX, float gridZ);
    
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getBasePosition() const; // NEW: Gets logical X/Z position, ignoring jump height
    bool getIsJumping() const { return isJumping; }
};

#endif