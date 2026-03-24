#ifndef CHICKEN_H
#define CHICKEN_H

#include <glm/glm.hpp>
#include "renderer.h"

class Chicken {
private:
    glm::vec3 position;

public:
    Chicken();
    void update(float deltaTime);
    void render(Renderer& renderer);
    void move(float dx, float dz);
    glm::vec3 getPosition() const { return position; }
};

#endif