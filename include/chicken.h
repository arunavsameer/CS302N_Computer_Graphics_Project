#ifndef CHICKEN_H
#define CHICKEN_H

#include <glm/glm.hpp>
#include "renderer.h"
#include "types.h"

class Chicken {
private:
    glm::vec3 position;
    
    // --- Jump State Variables ---
    bool isJumping;
    glm::vec3 startPos;
    glm::vec3 targetPos;
    float jumpProgress; 
    float rotationY;    

public:
    static constexpr float JUMP_DURATION = 0.25f; 
    static constexpr float JUMP_HEIGHT   = 1.5f;  

    Chicken();
    
    void update(float deltaTime);
    void render(Renderer& renderer);
    void move(float gridX, float gridZ);
    void applyLogVelocity(float velocityX, float deltaTime);
    
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getBasePosition() const; 
    glm::vec3 getSize() const { return glm::vec3(Config::CELL_SIZE * 0.8f); }
    bool getIsJumping() const { return isJumping; }
};

#endif