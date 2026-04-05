#include "../include/obstacle.h"

Obstacle::Obstacle(glm::vec3 startPos, float spd, ObstacleType t) 
    : position(startPos), speed(spd), type(t), isActive(true) {
    
    if (type == OBSTACLE_CAR) {
        size = glm::vec3(Config::CELL_SIZE * 1.5f, Config::CELL_SIZE * 0.8f, Config::CELL_SIZE * 0.8f);
    } else if (type == OBSTACLE_TRAIN) {
        size = glm::vec3(Config::TRAIN_LENGTH * Config::CELL_SIZE, Config::CELL_SIZE * 1.0f, Config::CELL_SIZE * 0.9f);
    } else if (type == OBSTACLE_LOG) {
        size = glm::vec3(Config::CELL_SIZE * 3.0f, Config::CELL_SIZE * 0.4f, Config::CELL_SIZE * 0.8f);
    }
}

void Obstacle::update(float deltaTime) {
    if (!isActive) return;

    position.x += speed * deltaTime;
    
    if (type == OBSTACLE_TRAIN) {
        // Trains pass once. If they go far off-screen, disable them entirely.
        if ((speed > 0 && position.x > 60.0f) || (speed < 0 && position.x < -60.0f)) {
            isActive = false; 
        }
    } else {
        // Cars and logs wrap around endlessly
        if (speed > 0 && position.x > 15.0f) position.x = -15.0f;
        if (speed < 0 && position.x < -15.0f) position.x = 15.0f;
    }
}

void Obstacle::render(Renderer& renderer) {
    if (!isActive) return;

    std::string tex;
    if (type == OBSTACLE_CAR) tex = "car";
    else if (type == OBSTACLE_TRAIN) tex = "train"; // Assumes you add train.png later
    else if (type == OBSTACLE_LOG) tex = "log";     // Assumes you add log.png later
    else tex = "road"; // Fallback texture

    // The size vector ensures it renders as a true cuboid matching the hitbox
    renderer.drawTexturedCube(position, size, tex);
}