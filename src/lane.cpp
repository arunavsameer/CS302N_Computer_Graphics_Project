#include "../include/lane.h"
#include "../include/types.h"

Lane::Lane(float z, LaneType t) : zPosition(z), type(t) {
    if (type == LANE_ROAD) {
        obstacles.push_back(Obstacle(glm::vec3(-5.0f, CELL_SIZE * 0.6f, zPosition), 5.0f, OBSTACLE_CAR));
    }
}

void Lane::update(float deltaTime) {
    for (auto& obs : obstacles) obs.update(deltaTime);
}

void Lane::render(Renderer& renderer) {
    std::string texName = (type == LANE_GRASS) ? "grass" : "road";
    
    // Draw the lane floor
    renderer.drawTexturedCube(glm::vec3(0.0f, 0.0f, zPosition), glm::vec3(30.0f, CELL_SIZE * 0.2f, CELL_SIZE), texName);
    
    for (auto& obs : obstacles) obs.render(renderer);
}