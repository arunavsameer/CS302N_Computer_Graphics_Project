#include "lane.h"

Lane::Lane(float z, LaneType t) : zPosition(z), type(t) {
    // Add a dummy obstacle to road lanes for testing
    if (type == LANE_ROAD) {
        obstacles.push_back(Obstacle(glm::vec3(-5.0f, 0.6f, zPosition), 5.0f, OBSTACLE_CAR));
    }
}

void Lane::update(float deltaTime) {
    for (auto& obs : obstacles) {
        obs.update(deltaTime);
    }
}

void Lane::render(Renderer& renderer) {
    glm::vec3 color = (type == LANE_GRASS) ? glm::vec3(0.2f, 0.8f, 0.2f) : glm::vec3(0.3f, 0.3f, 0.3f);
    renderer.drawLane(glm::vec3(0.0f, 0.0f, zPosition), 30.0f, color);
    
    for (auto& obs : obstacles) {
        obs.render(renderer);
    }
}