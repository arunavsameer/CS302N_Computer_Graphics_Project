#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
private:
    glm::vec3 position;
    glm::vec3 offset;

public:
    Camera();
    void update(int windowWidth, int windowHeight, glm::vec3 targetPos);
    void apply();
};

#endif