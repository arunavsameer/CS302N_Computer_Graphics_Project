#include "../include/obstacle.h"
#include "../include/types.h"

Obstacle::Obstacle(glm::vec3 startPos, float spd, ObstacleType t) : position(startPos), speed(spd), type(t) {}

void Obstacle::update(float deltaTime) {
    position.x += speed * deltaTime;
    if(position.x > 15.0f) position.x = -15.0f;
    if(position.x < -15.0f) position.x = 15.0f;
}

void Obstacle::render(Renderer& renderer) {
    if (type == OBSTACLE_CAR) {
        renderer.drawTexturedCube(position, glm::vec3(CELL_SIZE * 1.5f, CELL_SIZE * 0.8f, CELL_SIZE * 0.8f), "car");
    }
}