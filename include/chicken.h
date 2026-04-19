#ifndef CHICKEN_H
#define CHICKEN_H

#include <glm/glm.hpp>
#include <vector>
#include "renderer.h"
#include "types.h"

// ── Added Character Models ─────────────────────────────────────────────────
enum CharacterModel { MODEL_CHICKEN, MODEL_FROG };

// ── Water-death splatter particle ──────────────────────────────────────────
struct WaterParticle {
    glm::vec3 pos;
    glm::vec3 vel;
    float     size;
    glm::vec3 color;
    float     alpha;
};

enum DeathType { DEATH_NONE, DEATH_SQUISH, DEATH_WATER };

class Chicken {
private:
    glm::vec3 position;

    // --- Jump State ---
    bool      isJumping;
    glm::vec3 startPos;
    glm::vec3 targetPos;
    float     jumpProgress;
    float     rotationY;
    bool      isDead;

    // --- Character State ---
    CharacterModel currentModel;

    // --- Death Animation ---
    DeathType                  deathType;
    std::vector<WaterParticle> waterParticles;
    float                      waterSurfaceY;
    float                      waterDeathSinkTimer;  // pre-explosion sink phase
    bool                       waterDeathExploded;   // true once particles spawned

    void updateWaterParticles(float deltaTime);
    void spawnWaterDeathParticles();               // called after sink phase ends

public:
    static constexpr float JUMP_DURATION = 0.25f;
    static constexpr float JUMP_HEIGHT   = 0.75f;

    Chicken();

    void update(float deltaTime);
    void render(Renderer& renderer);
    void move(float gridX, float gridZ);
    void applyLogVelocity(float velocityX, float deltaTime);

    glm::vec3 getPosition()     const { return position; }
    glm::vec3 getBasePosition() const;
    glm::vec3 getSize()         const { return glm::vec3(Config::CELL_SIZE * 0.8f); }
    bool      getIsJumping()    const { return isJumping; }
    bool      getIsDead()       const { return isDead; }

    // --- New getters/setters for models ---
    void setModel(CharacterModel model) { currentModel = model; }
    CharacterModel getModel() const { return currentModel; }

    // Squish death (car / train)
    void setDead(bool dead) { isDead = dead; if (dead) deathType = DEATH_SQUISH; }

    // Water death – spawns block particles at surfaceY
    void triggerWaterDeath(float surfaceY);
    bool isWaterDeathFinished() const;

    void reset();
};

#endif