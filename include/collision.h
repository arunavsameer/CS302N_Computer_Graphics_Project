#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>

class Collision {
public:
    // Simple Axis-Aligned Bounding Box (AABB) check
    static bool checkAABB(glm::vec3 posA, glm::vec3 sizeA, glm::vec3 posB, glm::vec3 sizeB);
};

#endif