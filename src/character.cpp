#include "../include/character.h"
#include "../include/character_chicken.h"
#include "../include/character_frog.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

Chicken::Chicken() {
    setModel(MODEL_CHICKEN);
    reset();
}

void Chicken::setModel(CharacterModel model) {
    currentModel = model;
    if (model == MODEL_CHICKEN) {
        modelRenderer = std::make_unique<CharacterChicken>();
    } else if(model == MODEL_FROG) {
        modelRenderer = std::make_unique<CharacterFrog>();
    }
}

void Chicken::reset() {
    position            = glm::vec3(0.0f, Config::CELL_SIZE * 0.15f, 0.0f);
    isJumping           = false;
    isDead              = false;
    jumpProgress        = 0.0f;
    rotationY           = 180.0f;
    deathType           = DEATH_NONE;
    waterDeathSinkTimer = 0.0f;
    waterDeathExploded  = false;
    waterParticles.clear();
}

void Chicken::triggerWaterDeath(float surfaceY) {
    isDead              = true;
    deathType           = DEATH_WATER;
    waterSurfaceY       = surfaceY;
    waterDeathSinkTimer = 0.18f;
    waterDeathExploded  = false;
    waterParticles.clear();
}

void Chicken::spawnWaterDeathParticles() {
    std::vector<glm::vec3> palette = modelRenderer->getWaterDeathPalette();
    int numColors = palette.size();

    int count = 24 + rand() % 8;
    for (int i = 0; i < count; i++) {
        WaterParticle p;
        p.pos = position + glm::vec3(
            ((rand() % 200) - 100) * 0.007f,
            ((rand() % 80))        * 0.006f,
            ((rand() % 200) - 100) * 0.007f
        );

        float angle  = (rand() % 360) * 3.14159265f / 180.0f;
        float hspeed = 0.5f + (rand() % 100) * 0.022f;
        float vspeed = 1.4f + (rand() % 100) * 0.032f;

        if (i < count / 4) {
            vspeed *= 1.9f;
            hspeed *= 0.3f;
        }

        p.vel = glm::vec3(std::cos(angle) * hspeed, vspeed, std::sin(angle) * hspeed);
        p.size  = 0.07f + (rand() % 100) * 0.005f;
        p.color = palette[rand() % numColors];
        p.alpha = 1.0f;
        waterParticles.push_back(p);
    }
}

bool Chicken::isWaterDeathFinished() const {
    if (deathType != DEATH_WATER) return false;
    if (!waterDeathExploded) return false;
    for (const auto& p : waterParticles)
        if (p.alpha > 0.0f) return false;
    return true;
}

void Chicken::updateWaterParticles(float deltaTime) {
    for (auto& p : waterParticles) {
        if (p.alpha <= 0.0f) continue;

        p.vel.y -= 8.0f * deltaTime;
        p.pos   += p.vel * deltaTime;

        if (p.pos.y < waterSurfaceY) {
            float drag = 1.0f - 6.0f * deltaTime;
            if (drag < 0.0f) drag = 0.0f;
            p.vel.x *= drag;
            p.vel.z *= drag;
            if (p.vel.y > -0.6f) p.vel.y = -0.6f;

            p.alpha -= 2.8f * deltaTime;
            if (p.alpha < 0.0f) p.alpha = 0.0f;
        }
    }
}

void Chicken::update(float deltaTime) {
    if (deathType == DEATH_WATER) {
        if (!waterDeathExploded) {
            waterDeathSinkTimer -= deltaTime;
            position.y -= 1.8f * deltaTime;
            if (waterDeathSinkTimer <= 0.0f) {
                waterDeathExploded = true;
                spawnWaterDeathParticles();
            }
        } else {
            updateWaterParticles(deltaTime);
        }
        return;
    }

    if (!isJumping) return;

    jumpProgress += deltaTime / JUMP_DURATION;

    if (jumpProgress >= 1.0f) {
        position    = targetPos;
        isJumping   = false;
        jumpProgress = 0.0f;
    } else {
        position.x = startPos.x + (targetPos.x - startPos.x) * jumpProgress;
        position.z = startPos.z + (targetPos.z - startPos.z) * jumpProgress;
        float parabola = 4.0f * JUMP_HEIGHT * jumpProgress * (1.0f - jumpProgress);
        position.y = startPos.y + parabola;
    }
}

void Chicken::render(Renderer& renderer) {
    if (modelRenderer) {
        modelRenderer->render(renderer, *this);
    }
}

void Chicken::move(float gridX, float gridZ) {
    if (isJumping) return;
    if (gridX == 0.0f && gridZ == 0.0f) return;

    isJumping    = true;
    startPos     = position;
    targetPos    = position + glm::vec3(gridX * Config::CELL_SIZE, 0.0f, gridZ * Config::CELL_SIZE);
    jumpProgress = 0.0f;

    if      (gridZ < 0) rotationY = 180.0f;
    else if (gridZ > 0) rotationY =   0.0f;
    else if (gridX > 0) rotationY =  90.0f;
    else if (gridX < 0) rotationY = 270.0f;
}

glm::vec3 Chicken::getBasePosition() const {
    if (isJumping) {
        float baseY = startPos.y + (targetPos.y - startPos.y) * jumpProgress;
        return glm::vec3(position.x, baseY, position.z);
    }
    return position;
}

void Chicken::applyLogVelocity(float velocityX, float deltaTime) {
    float movement = velocityX * deltaTime;
    if (!isJumping) {
        position.x  += movement;
        startPos.x   = position.x;
        targetPos.x  = position.x;
    } else {
        startPos.x  += movement;
        targetPos.x += movement;
    }
}