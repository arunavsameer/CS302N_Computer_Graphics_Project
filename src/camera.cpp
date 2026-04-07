#include "../include/camera.h"
#include <GL/glew.h>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#include <cmath>
#include <algorithm>

Camera::Camera() {
    isLocked = false;
    currentPresetIndex = 0;
    overlayFadeTimer = 0.0f; // Start hidden

    // Presets
    presets.push_back({glm::radians(45.0f), glm::radians(45.0f), 11.3f});
    presets.push_back({0.0f, glm::radians(25.0f), 8.0f});
    presets.push_back({0.0f, glm::radians(85.0f), 18.0f});

    targetYaw = currentYaw = presets[0].yaw;
    targetPitch = currentPitch = presets[0].pitch;
    targetRadius = currentRadius = presets[0].radius;
}

void Camera::update(float deltaTime, int windowWidth, int windowHeight, glm::vec3 targetPos) {
    // Fade timer logic
    if (overlayFadeTimer > 0.0f) {
        overlayFadeTimer -= deltaTime;
    }

    currentYaw += (targetYaw - currentYaw) * params.lerpSpeed * deltaTime;
    currentPitch += (targetPitch - currentPitch) * params.lerpSpeed * deltaTime;
    currentRadius += (targetRadius - currentRadius) * params.lerpSpeed * deltaTime;

    offset.x = currentRadius * cos(currentPitch) * sin(currentYaw);
    offset.y = currentRadius * sin(currentPitch);
    offset.z = currentRadius * cos(currentPitch) * cos(currentYaw);

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
        position.x - offset.x, 0.0f, position.z - offset.z, 
        0.0f, 1.0f, 0.0f
    );
}

void Camera::renderOverlay(int windowWidth, int windowHeight) {
    if (overlayFadeTimer <= 0.0f) return;

    float alpha = std::min(1.0f, overlayFadeTimer);

    // Save current states
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

    // Save projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -100, 100);

    // Save modelview
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Setup safe state
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float padding = params.overlayRadius * 2.0f;
    glTranslatef(windowWidth - padding, windowHeight - padding, 0.0f);
    glRotatef(20.0f, 1.0f, 0.0f, 0.0f);

    // Sphere
    glColor4f(1.0f, 1.0f, 1.0f, alpha * 0.3f);
    glutWireSphere(params.overlayRadius, 12, 12);

    // Dot
    float dotX = params.overlayRadius * cos(currentPitch) * sin(currentYaw);
    float dotY = params.overlayRadius * sin(currentPitch);
    float dotZ = params.overlayRadius * cos(currentPitch) * cos(currentYaw);

    glTranslatef(dotX, dotY, dotZ);
    glColor4f(1.0f, 0.2f, 0.2f, alpha);
    glutSolidSphere(params.overlayRadius / 6.0f, 8, 8);

    // Restore matrices
    glPopMatrix(); // modelview
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Restore everything
    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
}

void Camera::processMouseDrag(float deltaX, float deltaY) {
    if (isLocked) return;

    targetYaw += deltaX * params.mouseSensitivity;
    targetPitch += deltaY * params.mouseSensitivity;
    targetPitch = std::max(params.minPitch, std::min(targetPitch, params.maxPitch));

    // Reset fade timer when moved
    overlayFadeTimer = params.overlayShowTime;
}

void Camera::cyclePreset() {
    currentPresetIndex = (currentPresetIndex + 1) % presets.size();
    targetYaw = presets[currentPresetIndex].yaw;
    targetPitch = presets[currentPresetIndex].pitch;
    targetRadius = presets[currentPresetIndex].radius;
    
    // Reset fade timer when preset changed
    overlayFadeTimer = params.overlayShowTime;
}

void Camera::toggleLock() {
    isLocked = !isLocked;
}


void Camera::setTargetRadius(float radius) {
    targetRadius = std::max(params.minRadius, std::min(radius, params.maxRadius));
}

void Camera::setLerpSpeed(float speed) {
    params.lerpSpeed = speed;
}

void Camera::resetToDefault() {
    params.lerpSpeed = 7.0f; // Reset to your original hyperparameter
    targetYaw = presets[currentPresetIndex].yaw;
    targetPitch = presets[currentPresetIndex].pitch;
    targetRadius = presets[currentPresetIndex].radius;
}