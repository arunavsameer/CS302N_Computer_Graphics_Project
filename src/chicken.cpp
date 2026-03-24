#include "chicken.h"
#include "types.h" 

Chicken::Chicken() {
    // Start sitting exactly on top of the ground cell
    position = glm::vec3(0.0f, CELL_SIZE * 0.6f, 0.0f); 
}

void Chicken::update(float deltaTime) {
    // We will add smooth hopping animation here later
}

void Chicken::render(Renderer& renderer) {
    // The chicken takes up 80% of a cell so it fits nicely inside the grid
    float size = CELL_SIZE * 0.8f; 
    renderer.drawCube(position, glm::vec3(size), glm::vec3(1.0f, 1.0f, 1.0f));
}

void Chicken::move(float gridX, float gridZ) {
    // Multiply the grid input (1 or -1) by the master cell size
    position.x += gridX * CELL_SIZE;
    position.z += gridZ * CELL_SIZE;
}