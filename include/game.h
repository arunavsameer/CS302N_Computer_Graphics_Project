#ifndef GAME_H
#define GAME_H

#include <vector>
#include "types.h"
#include "renderer.h"
#include "camera.h"
#include "chicken.h"
#include "lane.h"

class Game {
private:
    int windowWidth, windowHeight;
    GameState state;
    Renderer renderer;
    Camera camera;
    Chicken player;
    std::vector<Lane> lanes;
    float currentGenerationZ;
    float cameraTrackZ; // Moving world anchor for the camera and fail-behind check

    void generateLaneBlock();
    void checkCollisions(float deltaTime);
    void updateCameraAndFailState(float deltaTime);
    void maintainInfiniteLanes();

public:
    Game(int width, int height);
    void initialize();
    void update(float deltaTime);
    void render();
    void onKeyPress(unsigned char key);
    void onResize(int w, int h);
    void onMouseDrag(float deltaX, float deltaY);
};

#endif
