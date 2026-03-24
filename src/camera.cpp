#include "camera.h"
#include <GL/glew.h>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

Camera::Camera() {
    offset = glm::vec3(0.0f, 8.0f, 8.0f); // Isometric top-down angle
}

void Camera::update(int windowWidth, int windowHeight, glm::vec3 targetPos) {
    position = targetPos + offset;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)windowWidth / (double)windowHeight, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void Camera::apply() {
    glLoadIdentity();
    gluLookAt(
        position.x, position.y, position.z,
        position.x, 0.0f, position.z - offset.z, // Look slightly ahead of player
        0.0f, 1.0f, 0.0f
    );
}