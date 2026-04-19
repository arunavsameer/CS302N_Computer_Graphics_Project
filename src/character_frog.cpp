#include "../include/character_frog.h"

std::vector<glm::vec3> CharacterFrog::getWaterDeathPalette() const {
    return {
        {0.35f, 0.70f, 0.30f}, {0.35f, 0.70f, 0.30f}, {0.35f, 0.70f, 0.30f},
        {0.20f, 0.50f, 0.15f}, {0.20f, 0.50f, 0.15f},
        {0.85f, 0.90f, 0.70f},
        {1.00f, 1.00f, 1.00f},
        {0.05f, 0.05f, 0.05f}
    };
}

void CharacterFrog::drawModel(Renderer& renderer) const {
    glm::vec3 frogGreen = glm::vec3(0.35f, 0.70f, 0.30f);
    glm::vec3 frogDark  = glm::vec3(0.20f, 0.50f, 0.15f);
    glm::vec3 frogBelly = glm::vec3(0.85f, 0.90f, 0.70f);
    glm::vec3 white     = glm::vec3(1.00f, 1.00f, 1.00f);
    glm::vec3 black     = glm::vec3(0.05f, 0.05f, 0.05f);

    renderer.drawCube(glm::vec3(0.0f, 0.30f, 0.0f), glm::vec3(0.80f, 0.35f, 0.75f), frogGreen);
    renderer.drawCube(glm::vec3(0.0f, 0.20f, 0.05f), glm::vec3(0.82f, 0.20f, 0.70f), frogBelly);
    renderer.drawCube(glm::vec3(-0.25f, 0.55f, 0.20f), glm::vec3(0.25f, 0.25f, 0.25f), white);
    renderer.drawCube(glm::vec3( 0.25f, 0.55f, 0.20f), glm::vec3(0.25f, 0.25f, 0.25f), white);
    renderer.drawCube(glm::vec3(-0.25f, 0.55f, 0.33f), glm::vec3(0.10f, 0.10f, 0.05f), black);
    renderer.drawCube(glm::vec3( 0.25f, 0.55f, 0.33f), glm::vec3(0.10f, 0.10f, 0.05f), black);

    float legY = 0.10f;
    renderer.drawCube(glm::vec3(-0.45f, legY, -0.25f), glm::vec3(0.20f, 0.30f, 0.35f), frogDark);
    renderer.drawCube(glm::vec3( 0.45f, legY, -0.25f), glm::vec3(0.20f, 0.30f, 0.35f), frogDark);
    renderer.drawCube(glm::vec3(-0.40f, legY,  0.30f), glm::vec3(0.15f, 0.30f, 0.20f), frogDark);
    renderer.drawCube(glm::vec3( 0.40f, legY,  0.30f), glm::vec3(0.15f, 0.30f, 0.20f), frogDark);
}