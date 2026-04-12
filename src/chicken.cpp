#include "../include/chicken.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  Construction / Reset
// ─────────────────────────────────────────────────────────────────────────────
Chicken::Chicken() {
    position            = glm::vec3(0.0f, Config::CELL_SIZE * 0.15f, 0.0f);
    isJumping           = false;
    jumpProgress        = 0.0f;
    rotationY           = 180.0f;
    isDead              = false;
    deathType           = DEATH_NONE;
    waterSurfaceY       = 0.0f;
    waterDeathSinkTimer = 0.0f;
    waterDeathExploded  = false;
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

// ─────────────────────────────────────────────────────────────────────────────
//  Water death – Phase 1: brief visible sink, Phase 2: cube explosion
// ─────────────────────────────────────────────────────────────────────────────
void Chicken::triggerWaterDeath(float surfaceY) {
    isDead              = true;
    deathType           = DEATH_WATER;
    waterSurfaceY       = surfaceY;
    waterDeathSinkTimer = 0.18f;   // 0.18 s of visible sinking before explosion
    waterDeathExploded  = false;
    waterParticles.clear();
}

// Called automatically after the sink timer expires
void Chicken::spawnWaterDeathParticles() {
    // Palette mirrors the actual voxel colours of the chicken body
    static const glm::vec3 palette[] = {
        {1.00f, 1.00f, 1.00f},   // white  (body – highest weight)
        {1.00f, 1.00f, 1.00f},
        {1.00f, 1.00f, 1.00f},
        {1.00f, 0.50f, 0.05f},   // orange (beak / legs)
        {1.00f, 0.50f, 0.05f},
        {1.00f, 0.20f, 0.55f},   // hot-pink (comb)
        {0.85f, 0.10f, 0.10f},   // red (wattle)
        {0.05f, 0.05f, 0.05f},   // black (eyes)
    };
    const int numColors = 8;

    int count = 24 + rand() % 8;   // 24-31 cubes
    for (int i = 0; i < count; i++) {
        WaterParticle p;

        // Spawn scattered around the chicken centre
        p.pos = position + glm::vec3(
            ((rand() % 200) - 100) * 0.007f,
            ((rand() % 80))        * 0.006f,
            ((rand() % 200) - 100) * 0.007f
        );

        float angle  = (rand() % 360) * 3.14159265f / 180.0f;
        float hspeed = 0.5f + (rand() % 100) * 0.022f;
        float vspeed = 1.4f + (rand() % 100) * 0.032f;

        // A quarter of the cubes shoot straight up – "breaking apart" feel
        if (i < count / 4) {
            vspeed *= 1.9f;
            hspeed *= 0.3f;
        }

        p.vel = glm::vec3(
            std::cos(angle) * hspeed,
            vspeed,
            std::sin(angle) * hspeed
        );

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

// ─────────────────────────────────────────────────────────────────────────────
//  Per-frame water particle physics
// ─────────────────────────────────────────────────────────────────────────────
void Chicken::updateWaterParticles(float deltaTime) {
    for (auto& p : waterParticles) {
        if (p.alpha <= 0.0f) continue;

        // Gravity
        p.vel.y -= 8.0f * deltaTime;
        p.pos   += p.vel * deltaTime;

        if (p.pos.y < waterSurfaceY) {
            // Heavy drag once in water – kills horizontal momentum quickly
            float drag = 1.0f - 6.0f * deltaTime;
            if (drag < 0.0f) drag = 0.0f;
            p.vel.x *= drag;
            p.vel.z *= drag;

            // Steady sinking speed
            if (p.vel.y > -0.6f) p.vel.y = -0.6f;

            // Fade out in ~0.4 s after hitting surface
            p.alpha -= 2.8f * deltaTime;
            if (p.alpha < 0.0f) p.alpha = 0.0f;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Update
// ─────────────────────────────────────────────────────────────────────────────
void Chicken::update(float deltaTime) {
    // Water death: Phase 1 – sink visibly, Phase 2 – animate particles
    if (deathType == DEATH_WATER) {
        if (!waterDeathExploded) {
            // Sink the chicken body into the water surface
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

// ─────────────────────────────────────────────────────────────────────────────
//  Render
// ─────────────────────────────────────────────────────────────────────────────
void Chicken::render(Renderer& renderer) {

    // ── WATER DEATH: Phase 1 – sinking body; Phase 2 – cube particles ────────
    if (isDead && deathType == DEATH_WATER) {
        if (!waterDeathExploded) {
            // Render the chicken body normally – it will visibly translate down
            // because position.y is being decremented in update().
            // Fall through to the normal render below.
        } else {
            // Phase 2: draw flying / sinking cube particles only
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            for (const auto& p : waterParticles) {
                if (p.alpha <= 0.0f) continue;
                glPushMatrix();
                glTranslatef(p.pos.x, p.pos.y, p.pos.z);
                glScalef(p.size, p.size, p.size);
                glColor4f(p.color.r, p.color.g, p.color.b, p.alpha);
                glutSolidCube(1.0f);
                glPopMatrix();
            }

            glDisable(GL_BLEND);
            return;   // no chicken body drawn in phase 2
        }
    }

    // ── NORMAL / SQUISH render ───────────────────────────────────────────────
    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

    float s = Config::CELL_SIZE * 0.8f;

    if (isDead) {   // DEATH_SQUISH
        glTranslatef(0.0f, -0.05f * s, 0.0f);
        glScalef(1.2f, 0.15f, 1.2f);
    } else {
        glScalef(s, s, s);
    }

    glDisable(GL_TEXTURE_2D);

    // --- Palette ---
    glm::vec3 white   = glm::vec3(1.00f, 1.00f, 1.00f);
    glm::vec3 orange  = glm::vec3(1.00f, 0.50f, 0.05f);
    glm::vec3 hotPink = glm::vec3(1.00f, 0.20f, 0.55f);
    glm::vec3 red     = glm::vec3(0.85f, 0.10f, 0.10f);
    glm::vec3 black   = glm::vec3(0.05f, 0.05f, 0.05f);

    // Body
    renderer.drawCube(glm::vec3(0.0f,  0.50f,  0.0f),  glm::vec3(0.72f, 0.60f, 0.72f), white);
    // Head
    renderer.drawCube(glm::vec3(0.0f,  0.88f,  0.16f), glm::vec3(0.44f, 0.44f, 0.44f), white);
    // Comb
    renderer.drawCube(glm::vec3(0.0f,  1.13f,  0.16f), glm::vec3(0.16f, 0.26f, 0.22f), hotPink);
    // Beak
    renderer.drawCube(glm::vec3(0.0f,  0.86f,  0.42f), glm::vec3(0.13f, 0.10f, 0.16f), orange);
    // Wattle
    renderer.drawCube(glm::vec3(0.0f,  0.74f,  0.36f), glm::vec3(0.09f, 0.13f, 0.09f), red);
    // Eyes
    renderer.drawCube(glm::vec3(-0.23f, 0.92f, 0.30f), glm::vec3(0.07f, 0.09f, 0.05f), black);
    renderer.drawCube(glm::vec3( 0.23f, 0.92f, 0.30f), glm::vec3(0.07f, 0.09f, 0.05f), black);

    // Legs + feet
    float legX  = 0.20f;
    float legCY = 0.11f;
    float legH  = 0.22f;
    float legW  = 0.11f;
    renderer.drawCube(glm::vec3(-legX, legCY, 0.0f),  glm::vec3(legW, legH, legW),          orange);
    renderer.drawCube(glm::vec3(-legX, 0.03f, 0.10f), glm::vec3(0.24f, 0.06f, 0.28f),      orange);
    renderer.drawCube(glm::vec3( legX, legCY, 0.0f),  glm::vec3(legW, legH, legW),          orange);
    renderer.drawCube(glm::vec3( legX, 0.03f, 0.10f), glm::vec3(0.24f, 0.06f, 0.28f),      orange);

    // Drop shadow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
    glBegin(GL_QUADS);
        glVertex3f(-0.40f, 0.01f, -0.40f);
        glVertex3f( 0.40f, 0.01f, -0.40f);
        glVertex3f( 0.40f, 0.01f,  0.40f);
        glVertex3f(-0.40f, 0.01f,  0.40f);
    glEnd();
    glDisable(GL_BLEND);

    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Movement
// ─────────────────────────────────────────────────────────────────────────────
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