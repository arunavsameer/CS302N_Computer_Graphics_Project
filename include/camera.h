#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <vector>

// Hyperparameters for cam
struct CameraParams {
    float mouseSensitivity = 0.005f;
    float lerpSpeed = 7.0f;      
    float minPitch = 0.1f;       
    float maxPitch = 1.5f;       
    float minRadius = 3.0f;
    float maxRadius = 25.0f;
    float overlayShowTime = 2.0f;
    float overlayRadius = 40.0f;
};

struct CameraPreset {
    float yaw;
    float pitch;
    float radius;
};

class Camera {
private:
    glm::vec3 position;
    glm::vec3 offset;
    float followSpeed;

    float targetYaw, currentYaw;
    float targetPitch, currentPitch;
    float targetRadius, currentRadius;

    CameraParams params;
    bool isLocked;
    int currentPresetIndex;
    std::vector<CameraPreset> presets;

    // New variables for the HUD overlay
    float overlayFadeTimer;

public:
    Camera();
    void update(float deltaTime, int windowWidth, int windowHeight, glm::vec3 targetPos);
    void apply();
    
    void renderOverlay(int windowWidth, int windowHeight);

    void processMouseDrag(float deltaX, float deltaY);
    void cyclePreset();
    void toggleLock();

    void setTargetRadius(float radius);
    void setLerpSpeed(float speed);
    void resetToDefault();
    float getTargetRadius() const { return targetRadius; }
};

#endif