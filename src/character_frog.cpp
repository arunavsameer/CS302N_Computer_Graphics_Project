#include "../include/character_frog.h"
#include "../include/character.h" // Needed to read player.getDeathTimer()
#include <algorithm> // For std::min

std::vector<glm::vec3> CharacterFrog::getWaterDeathPalette() const {
    return {
        {0.35f, 0.70f, 0.30f}, {0.35f, 0.70f, 0.30f}, {0.35f, 0.70f, 0.30f},
        {0.20f, 0.50f, 0.15f}, {0.20f, 0.50f, 0.15f},
        {0.85f, 0.90f, 0.70f},
        {1.00f, 1.00f, 1.00f},
        {0.05f, 0.05f, 0.05f},
        {0.90f, 0.10f, 0.10f} 
    };
}

void CharacterFrog::drawModel(Renderer& renderer) const {
    glm::vec3 frogGreen = glm::vec3(0.35f, 0.70f, 0.30f);
    glm::vec3 frogDark  = glm::vec3(0.20f, 0.50f, 0.15f);
    glm::vec3 frogBelly = glm::vec3(0.85f, 0.90f, 0.70f);
    glm::vec3 white     = glm::vec3(1.00f, 1.00f, 1.00f);
    glm::vec3 black     = glm::vec3(0.05f, 0.05f, 0.05f);

    // Body & Belly
    renderer.drawCube(glm::vec3(0.0f, 0.30f, 0.0f), glm::vec3(0.80f, 0.35f, 0.75f), frogGreen);
    renderer.drawCube(glm::vec3(0.0f, 0.20f, 0.05f), glm::vec3(0.82f, 0.20f, 0.70f), frogBelly);
    
    // Eyes & Pupils
    renderer.drawCube(glm::vec3(-0.25f, 0.55f, 0.20f), glm::vec3(0.25f, 0.25f, 0.25f), white);
    renderer.drawCube(glm::vec3( 0.25f, 0.55f, 0.20f), glm::vec3(0.25f, 0.25f, 0.25f), white);
    renderer.drawCube(glm::vec3(-0.25f, 0.55f, 0.33f), glm::vec3(0.10f, 0.10f, 0.05f), black);
    renderer.drawCube(glm::vec3( 0.25f, 0.55f, 0.33f), glm::vec3(0.10f, 0.10f, 0.05f), black);

    // Legs
    float legY = 0.10f;
    renderer.drawCube(glm::vec3(-0.45f, legY, -0.25f), glm::vec3(0.20f, 0.30f, 0.35f), frogDark);
    renderer.drawCube(glm::vec3( 0.45f, legY, -0.25f), glm::vec3(0.20f, 0.30f, 0.35f), frogDark);
    renderer.drawCube(glm::vec3(-0.40f, legY,  0.30f), glm::vec3(0.15f, 0.30f, 0.20f), frogDark);
    renderer.drawCube(glm::vec3( 0.40f, legY,  0.30f), glm::vec3(0.15f, 0.30f, 0.20f), frogDark);
}

void CharacterFrog::drawExtra(Renderer& renderer, const Chicken& player) const {
    glm::vec3 red = glm::vec3(0.90f, 0.10f, 0.10f);

    float zOffset = 0.45f; // Base position (sticks out mouth)
    float zSize   = 0.15f; // Reduced base size so it starts smaller
    
    if (player.getIsDead()) {
        float t = player.getDeathTimer();
        
        // Multiply timer by speed. Max out at 1.0. 
        float extensionProgress = std::min(t * 4.0f, 1.0f); 
        
        // DECREASED MAX LENGTH: Now only extends up to 0.8 units instead of 2.0
        float extraLength = extensionProgress * 0.8f; 
        
        zSize += extraLength;
        // Shift center forward by half the added length
        zOffset += extraLength / 2.0f; 
    }

    // DECREASED WIDTH & HEIGHT: Made the tongue narrower (0.08f) and thinner (0.03f)
    renderer.drawCube(glm::vec3(0.0f, 0.25f, zOffset), glm::vec3(0.08f, 0.03f, zSize), red);
}