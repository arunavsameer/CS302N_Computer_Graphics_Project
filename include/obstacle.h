#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <glm/glm.hpp>
#include "types.h"
#include "renderer.h"

class Obstacle {
private:
    glm::vec3 position;
    float speed;
    ObstacleType type;

public:
    Obstacle(glm::vec3 startPos, float spd, ObstacleType t);
    void update(float deltaTime);
    void render(Renderer& renderer);
    glm::vec3 getPosition() const { return position; }
};

#endif