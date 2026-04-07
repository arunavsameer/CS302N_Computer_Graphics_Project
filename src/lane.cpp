#include "../include/lane.h"
#include <cstdlib>

Lane::Lane(float z, LaneType t) : zPosition(z), type(t) {
    float dir = (rand() % 2 == 0) ? 1.0f : -1.0f;

    if (type == LANE_ROAD) {
        float speed = dir * (Config::CAR_SPEED_MIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(Config::CAR_SPEED_MAX - Config::CAR_SPEED_MIN))));
        float startX = dir * -(5.0f + static_cast<float>(rand() % 10));

        int vehiclePick = rand() % 3;
        VehicleVariant variant = VEHICLE_SMALL_CAR;
        if (vehiclePick == 1) variant = VEHICLE_BIG_CAR;
        else if (vehiclePick == 2) variant = VEHICLE_TRUCK;

        obstacles.push_back(Obstacle(glm::vec3(startX, Config::CELL_SIZE * 0.6f, zPosition), speed, OBSTACLE_CAR, variant));
    } 
    else if (type == LANE_RAIL) {
        float speed = dir * Config::TRAIN_SPEED;
        float startX = dir * -(30.0f + static_cast<float>(rand() % 50));
        obstacles.push_back(Obstacle(glm::vec3(startX, Config::CELL_SIZE * 0.6f, zPosition), speed, OBSTACLE_TRAIN));
    }
    else if (type == LANE_RIVER) {
        float speed = dir * (Config::LOG_SPEED_MIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(Config::LOG_SPEED_MAX - Config::LOG_SPEED_MIN))));
        obstacles.push_back(Obstacle(glm::vec3(-5.0f, Config::CELL_SIZE * 0.2f, zPosition), speed, OBSTACLE_LOG));
        obstacles.push_back(Obstacle(glm::vec3(5.0f, Config::CELL_SIZE * 0.2f, zPosition), speed, OBSTACLE_LOG));
    }

    if (type == LANE_GRASS && rand() % 5 == 0) {
        float x = (rand() % 5 - 2) * Config::CELL_SIZE;
        coins.emplace_back(glm::vec3(x, Config::CELL_SIZE * 0.6f, zPosition));
    }
}

void Lane::update(float deltaTime) {
    for (auto& obs : obstacles) obs.update(deltaTime);
}

void Lane::render(Renderer& renderer) {
    std::string texName;
    
    if (type == LANE_GRASS) {
        int gridZ = std::abs(static_cast<int>(std::round(zPosition / Config::CELL_SIZE)));
        texName = (gridZ % 2 == 0) ? "grass" : "grass2";
    }
    else if (type == LANE_ROAD) texName = "road";
    else if (type == LANE_RAIL) texName = "rail"; 
    else if (type == LANE_RIVER) texName = "river"; 
    else texName = "grass";
    
    float yPos = (type == LANE_RIVER) ? -Config::CELL_SIZE * 0.2f : 0.0f;
    renderer.drawTexturedCube(glm::vec3(0.0f, yPos, zPosition), glm::vec3(30.0f, Config::CELL_SIZE * 0.2f, Config::CELL_SIZE), texName);
    
    for (auto& obs : obstacles) obs.render(renderer);
    for (auto& coin : coins) {
        coin.render(renderer);
    }
}