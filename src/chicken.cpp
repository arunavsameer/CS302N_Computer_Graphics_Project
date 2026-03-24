#include "../include/chicken.h"
#include "../include/types.h"

Chicken::Chicken() { position = glm::vec3(0.0f, CELL_SIZE * 0.6f, 0.0f); }
void Chicken::update(float deltaTime) {}

void Chicken::render(Renderer& renderer) {
    // Uses the "chicken" texture we loaded in the renderer
    renderer.drawTexturedCube(position, glm::vec3(CELL_SIZE * 0.8f), "chicken");
}

void Chicken::move(float gridX, float gridZ) {
    position.x += gridX * CELL_SIZE;
    position.z += gridZ * CELL_SIZE;
}