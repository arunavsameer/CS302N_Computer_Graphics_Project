#include "../include/collision.h"
#include "../include/types.h"
#include <cmath>

bool Collision::checkAABB(glm::vec3 posA, glm::vec3 sizeA, glm::vec3 posB, glm::vec3 sizeB) {
    // Shrink the physical bounds slightly compared to visual bounds for fair gameplay
    glm::vec3 hitA = sizeA * Config::HITBOX_PADDING;
    glm::vec3 hitB = sizeB * Config::HITBOX_PADDING;

    return (std::abs(posA.x - posB.x) < (hitA.x + hitB.x) / 2.0f) &&
           (std::abs(posA.y - posB.y) < (hitA.y + hitB.y) / 2.0f) &&
           (std::abs(posA.z - posB.z) < (hitA.z + hitB.z) / 2.0f);
}