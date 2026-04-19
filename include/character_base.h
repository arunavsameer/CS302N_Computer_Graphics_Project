#ifndef CHARACTER_BASE_H
#define CHARACTER_BASE_H

#include "renderer.h"
#include <glm/glm.hpp>
#include <vector>

// Forward declaration of Chicken to access its state
class Chicken;

// Shared character rendering behavior
class CharacterBase {
public:
    virtual ~CharacterBase() = default;

    // Standard render pipeline: transforms, shadow, death states
    void render(Renderer& renderer, const Chicken& player);

    // Each character provides its own water death colors
    virtual std::vector<glm::vec3> getWaterDeathPalette() const = 0;

protected:
    // Specific characters implement this to draw their actual body cubes
    virtual void drawModel(Renderer& renderer) const = 0;
};

#endif