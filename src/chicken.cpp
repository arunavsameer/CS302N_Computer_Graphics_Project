#include "../include/chicken.h"
#include <iostream>
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif


Chicken::Chicken() { 
    position = glm::vec3(0.0f, Config::CELL_SIZE * 0.15f, 0.0f); 
    isJumping = false;
    jumpProgress = 0.0f;
    rotationY = 180.0f; 
    isDead = false;
}

void Chicken::reset() {
    position = glm::vec3(0.0f, Config::CELL_SIZE * 0.15f, 0.0f);
    isJumping = false;
    isDead = false;
    jumpProgress = 0.0f;
    rotationY = 180.0f;
}

void Chicken::update(float deltaTime) {
    if (!isJumping) return;

    jumpProgress += deltaTime / JUMP_DURATION;

    if (jumpProgress >= 1.0f) {
        position = targetPos;
        isJumping = false;
        jumpProgress = 0.0f;
    } else {
        position.x = startPos.x + (targetPos.x - startPos.x) * jumpProgress;
        position.z = startPos.z + (targetPos.z - startPos.z) * jumpProgress;
        
        float parabola = 4.0f * JUMP_HEIGHT * jumpProgress * (1.0f - jumpProgress);
        position.y = startPos.y + parabola;
    }
}

void Chicken::render(Renderer& renderer) {
    glPushMatrix();
    
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    
    float s = Config::CELL_SIZE * 0.8f;
    
    if (isDead) {
        glTranslatef(0.0f, -0.05f * s, 0.0f); 
        glScalef(1.2f, 0.15f, 1.2f);
    } else {
        glScalef(s, s, s);
    }
    
    glDisable(GL_TEXTURE_2D);

    // --- Palette (matching the reference Crossy Road chicken) ---
    glm::vec3 white    = glm::vec3(1.00f, 1.00f, 1.00f);
    glm::vec3 orange   = glm::vec3(1.00f, 0.50f, 0.05f);  // beak, legs, feet
    glm::vec3 hotPink  = glm::vec3(1.00f, 0.20f, 0.55f);  // comb (pink like the image)
    glm::vec3 red      = glm::vec3(0.85f, 0.10f, 0.10f);  // wattle
    glm::vec3 black    = glm::vec3(0.05f, 0.05f, 0.05f);  // eyes

    // =====================================================================
    // BODY  –  tall white block, slightly raised so legs poke out below
    // Body spans y: 0.20 → 0.80  (center=0.50, half-height=0.30 * scale 0.60)
    // =====================================================================
    renderer.drawCube(glm::vec3(0.0f,  0.50f,  0.0f),  glm::vec3(0.72f, 0.60f, 0.72f), white);

    // =====================================================================
    // HEAD  –  slightly smaller cube sitting on the front-top of the body
    // Faces +Z direction (beak points toward +Z in model space)
    // =====================================================================
    renderer.drawCube(glm::vec3(0.0f,  0.88f,  0.16f), glm::vec3(0.44f, 0.44f, 0.44f), white);

    // =====================================================================
    // COMB  –  hot-pink block on top of head
    // =====================================================================
    renderer.drawCube(glm::vec3(0.0f,  1.13f,  0.16f), glm::vec3(0.16f, 0.26f, 0.22f), hotPink);

    // =====================================================================
    // BEAK  –  ORANGE (not yellow!)
    // =====================================================================
    renderer.drawCube(glm::vec3(0.0f,  0.86f,  0.42f), glm::vec3(0.13f, 0.10f, 0.16f), orange);

    // =====================================================================
    // WATTLE  –  small red blob below the beak
    // =====================================================================
    renderer.drawCube(glm::vec3(0.0f,  0.74f,  0.36f), glm::vec3(0.09f, 0.13f, 0.09f), red);

    // =====================================================================
    // EYES  –  two small black cubes on the left and right faces of the head
    //          z ~= 0.30 puts them on the front quarter of the head
    // =====================================================================
    renderer.drawCube(glm::vec3(-0.23f, 0.92f, 0.30f), glm::vec3(0.07f, 0.09f, 0.05f), black);
    renderer.drawCube(glm::vec3( 0.23f, 0.92f, 0.30f), glm::vec3(0.07f, 0.09f, 0.05f), black);

    // =====================================================================
    // LEGS  –  placed at x = ±0.20 so they CLEAR the body sides and show
    //          Body bottom edge is at y = 0.50 - 0.30 = 0.20
    //          Legs span y: 0.0 → 0.22  → clearly visible below the body
    // =====================================================================
    float legX   = 0.20f;
    float legCY  = 0.11f;   // centre of leg cube
    float legH   = 0.22f;   // height
    float legW   = 0.11f;   // thickness

    // Left leg
    renderer.drawCube(glm::vec3(-legX,  legCY, 0.0f),  glm::vec3(legW, legH, legW), orange);
    // Left foot  (wider, points forward)
    renderer.drawCube(glm::vec3(-legX,  0.03f, 0.10f), glm::vec3(0.24f, 0.06f, 0.28f), orange);

    // Right leg
    renderer.drawCube(glm::vec3( legX,  legCY, 0.0f),  glm::vec3(legW, legH, legW), orange);
    // Right foot
    renderer.drawCube(glm::vec3( legX,  0.03f, 0.10f), glm::vec3(0.24f, 0.06f, 0.28f), orange);

    // =====================================================================
    // DROP SHADOW
    // =====================================================================
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

void Chicken::move(float gridX, float gridZ) {
    if (isJumping) return; 
    if (gridX == 0.0f && gridZ == 0.0f) return;

    isJumping = true;
    startPos = position;
    targetPos = position + glm::vec3(gridX * Config::CELL_SIZE, 0.0f, gridZ * Config::CELL_SIZE);
    jumpProgress = 0.0f;

    // -----------------------------------------------------------------
    // The chicken model faces +Z at rotationY = 0.
    // OpenGL glRotatef(angle, 0,1,0) maps +Z → +X at 90°, +Z → -Z at 180°
    //
    //   W  (gridZ = -1) → move toward -Z  → face -Z  → rotationY = 180
    //   S  (gridZ = +1) → move toward +Z  → face +Z  → rotationY =   0
    //   D  (gridX = +1) → move toward +X  → face +X  → rotationY =  90
    //   A  (gridX = -1) → move toward -X  → face -X  → rotationY = 270
    // -----------------------------------------------------------------
    if      (gridZ < 0) rotationY = 180.0f;   // W – forward
    else if (gridZ > 0) rotationY =   0.0f;   // S – backward
    else if (gridX > 0) rotationY =  90.0f;   // D – right
    else if (gridX < 0) rotationY = 270.0f;   // A – left
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
        position.x += movement;
        startPos.x = position.x; 
        targetPos.x = position.x;
    } else {
        startPos.x += movement;
        targetPos.x += movement;
    }
}