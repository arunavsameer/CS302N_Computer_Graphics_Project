#include "../include/lane.h"
#include <cstdlib>

Lane::Lane(float z, LaneType t) : zPosition(z), type(t) {
    float dir = (rand() % 2 == 0) ? 1.0f : -1.0f;

    if (type == LANE_ROAD) {
        float speed = dir * (Config::CAR_SPEED_MIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(Config::CAR_SPEED_MAX - Config::CAR_SPEED_MIN))));
        // Offset cars randomly so they don't look perfectly aligned
        float startX = dir * -(5.0f + static_cast<float>(rand() % 10));
        obstacles.push_back(Obstacle(glm::vec3(startX, Config::CELL_SIZE * 0.6f, zPosition), speed, OBSTACLE_CAR));
    } 
    else if (type == LANE_RAIL) {
        float speed = dir * Config::TRAIN_SPEED;
        // Start trains very far away randomly (30 to 80 units off-screen). 
        // It acts as a delayed hazard that only sweeps across the map once.
        float startX = dir * -(30.0f + static_cast<float>(rand() % 50));
        obstacles.push_back(Obstacle(glm::vec3(startX, Config::CELL_SIZE * 0.6f, zPosition), speed, OBSTACLE_TRAIN));
    }
    else if (type == LANE_RIVER) {
        float speed = dir * (Config::LOG_SPEED_MIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(Config::LOG_SPEED_MAX - Config::LOG_SPEED_MIN))));
        obstacles.push_back(Obstacle(glm::vec3(-5.0f, Config::CELL_SIZE * 0.2f, zPosition), speed, OBSTACLE_LOG));
        obstacles.push_back(Obstacle(glm::vec3(5.0f, Config::CELL_SIZE * 0.2f, zPosition), speed, OBSTACLE_LOG));
    }
}

void Lane::update(float deltaTime) {
    for (auto& obs : obstacles) obs.update(deltaTime);
}

void Lane::render(Renderer& renderer) {
    std::string texName;
    if (type == LANE_GRASS) texName = "grass";
    else if (type == LANE_ROAD) texName = "road";
    else if (type == LANE_RAIL) texName = "rail"; 
    else if (type == LANE_RIVER) texName = "river"; 
    else texName = "grass";
    
    float yPos = (type == LANE_RIVER) ? -Config::CELL_SIZE * 0.2f : 0.0f;
    renderer.drawTexturedCube(glm::vec3(0.0f, yPos, zPosition), glm::vec3(30.0f, Config::CELL_SIZE * 0.2f, Config::CELL_SIZE), texName);
    
    for (auto& obs : obstacles) obs.render(renderer);
}