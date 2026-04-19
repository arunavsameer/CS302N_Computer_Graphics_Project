#include "../include/character_base.h"
#include "../include/chicken.h"
#include "../include/types.h"

#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

void CharacterBase::render(Renderer& renderer, const Chicken& player) {
    bool isDead = player.getIsDead();
    int deathType = player.getDeathType();
    bool waterDeathExploded = player.getWaterDeathExploded();
    const auto& waterParticles = player.getWaterParticles();
    glm::vec3 position = player.getPosition();
    float rotationY = player.getRotationY();

    // ── WATER DEATH: Phase 1 – sinking body; Phase 2 – cube particles ────────
    if (isDead && deathType == 2) { // 2 = DEATH_WATER
        if (waterDeathExploded) {
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
            glEnable(GL_TEXTURE_2D);
            return;
        }
    }

    // ── NORMAL / SQUISH render ───────────────────────────────────────────────
    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

    float s = Config::CELL_SIZE * 0.8f;

    if (isDead) {
        if (deathType != 2) { // DEATH_SQUISH
            glTranslatef(0.0f, -0.05f * s, 0.0f);
            glScalef(1.2f, 0.15f, 1.2f);
        } else {
            glScalef(0.0f, 0.0f, 0.0f);
        }
    } else {
        glScalef(s, s, s);
    }

    glDisable(GL_TEXTURE_2D);

    // Call the specific character's geometry
    drawModel(renderer);

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
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
}