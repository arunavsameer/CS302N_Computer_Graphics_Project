#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <glm/glm.hpp>
#include "types.h"
#include "renderer.h"

class Obstacle {
private:
    glm::vec3 position;
    glm::vec3 size;
    float speed;
    ObstacleType type;
    bool isActive; // Tracks if the obstacle is still in play (used for trains)

public:
    Obstacle(glm::vec3 startPos, float spd, ObstacleType t);
    void update(float deltaTime);
    void render(Renderer& renderer);
    
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getSize() const { return size; }
    ObstacleType getType() const { return type; }
    float getSpeed() const { return speed; }
    bool getIsActive() const { return isActive; }
};

#endif