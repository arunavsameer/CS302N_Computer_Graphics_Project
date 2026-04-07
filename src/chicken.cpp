#include "../include/chicken.h"
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif


Chicken::Chicken() { 
    position = glm::vec3(0.0f, Config::CELL_SIZE * 0.6f, 0.0f); 
    isJumping = false;
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
    glm::vec3 pos = position;
    float s = Config::CELL_SIZE;
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    glRotatef(rotationY, 0, 1, 0);
    glTranslatef(-pos.x, -pos.y, -pos.z);
    // glUseProgram(0);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    // ===== BODY =====
    renderer.drawCube(pos + glm::vec3(0, 0.3f*s, 0),
                      glm::vec3(0.6f*s),
                      glm::vec3(1.0f, 1.0f, 1.0f));

    // ===== HEAD =====
    renderer.drawCube(pos + glm::vec3(0, 0.75f*s, 0),
                      glm::vec3(0.35f*s),
                      glm::vec3(1.0f, 1.0f, 1.0f));

    // ===== BEAK =====
    renderer.drawCube(pos + glm::vec3(0.25f*s, 0.75f*s, 0),
                      glm::vec3(0.2f*s, 0.15f*s, 0.2f*s),
                      glm::vec3(1.0f, 0.5f, 0.2f));

    // ===== CREST =====
    renderer.drawCube(pos + glm::vec3(0, 0.95f*s, 0),
                      glm::vec3(0.25f*s, 0.15f*s, 0.25f*s),
                      glm::vec3(1.0f, 0.2f, 0.4f));

    // ===== WING =====
    renderer.drawCube(pos + glm::vec3(-0.35f*s, 0.4f*s, 0),
                      glm::vec3(0.25f*s, 0.3f*s, 0.4f*s),
                      glm::vec3(0.95f, 0.95f, 0.95f));

    // ===== LEGS =====
    renderer.drawCube(pos + glm::vec3(-0.1f*s, 0.05f*s, 0),
                      glm::vec3(0.1f*s, 0.3f*s, 0.1f*s),
                      glm::vec3(1.0f, 0.6f, 0.2f));

    renderer.drawCube(pos + glm::vec3(0.1f*s, 0.05f*s, 0),
                      glm::vec3(0.1f*s, 0.3f*s, 0.1f*s),
                      glm::vec3(1.0f, 0.6f, 0.2f));

    // ===== FEET =====
    renderer.drawCube(pos + glm::vec3(-0.1f*s, -0.05f*s, 0.1f*s),
                      glm::vec3(0.25f*s, 0.05f*s, 0.25f*s),
                      glm::vec3(1.0f, 0.6f, 0.2f));

    renderer.drawCube(pos + glm::vec3(0.1f*s, -0.05f*s, 0.1f*s),
                      glm::vec3(0.25f*s, 0.05f*s, 0.25f*s),
                      glm::vec3(1.0f, 0.6f, 0.2f));
   
    // ===== SHADOW (INSIDE FUNCTION) =====
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
    glBegin(GL_QUADS);
        glVertex3f(pos.x - 0.3f, pos.y, pos.z - 0.3f);
        glVertex3f(pos.x + 0.3f, pos.y, pos.z - 0.3f);
        glVertex3f(pos.x + 0.3f, pos.y, pos.z + 0.3f);
        glVertex3f(pos.x - 0.3f, pos.y, pos.z + 0.3f);
    glEnd();
    glPopMatrix();
}

void Chicken::move(float gridX, float gridZ) {
    if (isJumping) return; 
    if (gridX == 0.0f && gridZ == 0.0f) return;

    isJumping = true;
    startPos = position;
    targetPos = position + glm::vec3(gridX * Config::CELL_SIZE, 0.0f, gridZ * Config::CELL_SIZE);
    jumpProgress = 0.0f;
    if (gridZ < 0) rotationY = 90.0f;      // forward
else if (gridZ > 0) rotationY = -90.0f; // backward
else if (gridX > 0) rotationY = 0.0f;   // right
else if (gridX < 0) rotationY = 180.0f; // left


}

glm::vec3 Chicken::getBasePosition() const {
    if (isJumping) {
        float baseY = startPos.y + (targetPos.y - startPos.y) * jumpProgress;
        return glm::vec3(position.x, baseY, position.z);
    }
    return position;
}

void Chicken::applyLogVelocity(float velocityX, float deltaTime) {
    if (!isJumping) {
        position.x += velocityX * deltaTime;
        startPos.x = position.x; 
        targetPos.x = position.x;
    }
}
