#ifndef COIN_H
#define COIN_H

#include <glm/glm.hpp>
#include "renderer.h"

class Coin {
public:
    Coin(glm::vec3 pos);

    void render(Renderer& renderer);
    glm::vec3 getPosition() const;
    float getSize() const;

    bool collected;

private:
    glm::vec3 position;   // 🔥 THIS WAS MISSING
};

#endif