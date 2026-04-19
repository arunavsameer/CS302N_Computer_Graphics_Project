#include "../include/character_chicken.h"

std::vector<glm::vec3> CharacterChicken::getWaterDeathPalette() const {
    return {
        {1.00f, 1.00f, 1.00f}, {1.00f, 1.00f, 1.00f}, {1.00f, 1.00f, 1.00f},
        {1.00f, 0.50f, 0.05f}, {1.00f, 0.50f, 0.05f},
        {1.00f, 0.20f, 0.55f},
        {0.85f, 0.10f, 0.10f},
        {0.05f, 0.05f, 0.05f}
    };
}

void CharacterChicken::drawModel(Renderer& renderer) const {
    glm::vec3 white   = glm::vec3(1.00f, 1.00f, 1.00f);
    glm::vec3 orange  = glm::vec3(1.00f, 0.50f, 0.05f);
    glm::vec3 hotPink = glm::vec3(1.00f, 0.20f, 0.55f);
    glm::vec3 red     = glm::vec3(0.85f, 0.10f, 0.10f);
    glm::vec3 black   = glm::vec3(0.05f, 0.05f, 0.05f);

    renderer.drawCube(glm::vec3(0.0f,  0.50f,  0.0f),  glm::vec3(0.72f, 0.60f, 0.72f), white);
    renderer.drawCube(glm::vec3(0.0f,  0.88f,  0.16f), glm::vec3(0.44f, 0.44f, 0.44f), white);
    renderer.drawCube(glm::vec3(0.0f,  1.13f,  0.16f), glm::vec3(0.16f, 0.26f, 0.22f), hotPink);
    renderer.drawCube(glm::vec3(0.0f,  0.86f,  0.42f), glm::vec3(0.13f, 0.10f, 0.16f), orange);
    renderer.drawCube(glm::vec3(0.0f,  0.74f,  0.36f), glm::vec3(0.09f, 0.13f, 0.09f), red);
    renderer.drawCube(glm::vec3(-0.23f, 0.92f, 0.30f), glm::vec3(0.07f, 0.09f, 0.05f), black);
    renderer.drawCube(glm::vec3( 0.23f, 0.92f, 0.30f), glm::vec3(0.07f, 0.09f, 0.05f), black);

    float legX  = 0.20f, legCY = 0.11f, legH  = 0.22f, legW  = 0.11f;
    renderer.drawCube(glm::vec3(-legX, legCY, 0.0f),  glm::vec3(legW, legH, legW), orange);
    renderer.drawCube(glm::vec3(-legX, 0.03f, 0.10f), glm::vec3(0.24f, 0.06f, 0.28f), orange);
    renderer.drawCube(glm::vec3( legX, legCY, 0.0f),  glm::vec3(legW, legH, legW), orange);
    renderer.drawCube(glm::vec3( legX, 0.03f, 0.10f), glm::vec3(0.24f, 0.06f, 0.28f), orange);
}