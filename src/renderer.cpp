#include "renderer.h"
#include "types.h"
#include <GL/glew.h>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

void Renderer::initialize() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos[] = { 5.0f, 10.0f, 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glClearColor(0.4f, 0.7f, 1.0f, 1.0f); // Sky blue
}

void Renderer::prepareFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef(scale.x, scale.y, scale.z);
    glColor3f(color.r, color.g, color.b);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void Renderer::drawLane(glm::vec3 position, float width, glm::vec3 color) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    
    glScalef(width, CELL_SIZE * 0.2f, CELL_SIZE); 
    
    glColor3f(color.r, color.g, color.b);
    glutSolidCube(1.0f);
    glPopMatrix();
}