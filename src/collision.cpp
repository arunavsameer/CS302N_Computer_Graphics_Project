#include "collision.h"
#include <cmath>

bool Collision::checkAABB(glm::vec3 posA, glm::vec3 sizeA, glm::vec3 posB, glm::vec3 sizeB) {
    return (std::abs(posA.x - posB.x) < (sizeA.x + sizeB.x) / 2.0f) &&
           (std::abs(posA.y - posB.y) < (sizeA.y + sizeB.y) / 2.0f) &&
           (std::abs(posA.z - posB.z) < (sizeA.z + sizeB.z) / 2.0f);
}