#include "../include/coin.h"
#define GL_SILENCE_DEPRECATION

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
Coin::Coin(glm::vec3 pos) {
    position = pos;
    collected = false;
}

void Coin::render(Renderer& renderer) {
    if (collected) return;

    glDisable(GL_TEXTURE_2D);
    // glUseProgram(0); // disable shader
    glColor3f(1.0f, 0.85f, 0.0f); // gold color

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);

    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glRotatef(time * 120.0f, 0, 1, 0);

    glutSolidSphere(0.2f, 20, 20);

    glPopMatrix();
}

glm::vec3 Coin::getPosition() const { return position; }
float Coin::getSize() const { return 0.3f; }