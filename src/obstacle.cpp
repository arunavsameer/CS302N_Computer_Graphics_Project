#include "obstacle.h"

Obstacle::Obstacle(glm::vec3 startPos, float spd, ObstacleType t) 
    : position(startPos), speed(spd), type(t) {}

void Obstacle::update(float deltaTime) {
    position.x += speed * deltaTime;
    // Reset if it goes too far off screen
    if(position.x > 15.0f) position.x = -15.0f;
    if(position.x < -15.0f) position.x = 15.0f;
}

void Obstacle::render(Renderer& renderer) {
    glm::vec3 color = (type == OBSTACLE_CAR) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.5f, 0.3f, 0.1f);
    renderer.drawCube(position, glm::vec3(1.5f, 0.8f, 0.8f), color);
}