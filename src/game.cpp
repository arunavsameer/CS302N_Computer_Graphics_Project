#include "../include/game.h"

Game::Game(int width, int height) : windowWidth(width), windowHeight(height), state(GAME_STATE_PLAYING) {}

void Game::initialize() {
    renderer.initialize();
    
    for (int i = 5; i >= -15; i--) {
        LaneType type = (i % 3 == 0) ? LANE_ROAD : LANE_GRASS;
        lanes.push_back(Lane((float)i * CELL_SIZE, type)); 
    }
}

void Game::update(float deltaTime) {
    if (state != GAME_STATE_PLAYING) return;

    player.update(deltaTime);
    
    // CHANGED: Pass the base position so the camera doesn't wiggle on the Y-axis
    camera.update(deltaTime, windowWidth, windowHeight, player.getBasePosition());
    
    for (auto& lane : lanes) {
        lane.update(deltaTime);
    }
}

void Game::render() {
    renderer.prepareFrame();
    camera.apply();

    for (auto& lane : lanes) {
        lane.render(renderer);
    }
    player.render(renderer);

    // Draw the UI overlay last
    camera.renderOverlay(windowWidth, windowHeight);
}

void Game::onKeyPress(unsigned char key) {
    if (state != GAME_STATE_PLAYING) return;

    if(key == 'w' || key == 'W') player.move(0.0f, -1.0f);
    if(key == 's' || key == 'S') player.move(0.0f, 1.0f);
    if(key == 'a' || key == 'A') player.move(-1.0f, 0.0f);
    if(key == 'd' || key == 'D') player.move(1.0f, 0.0f);
    
    // Camera Controls
    if(key == 'v' || key == 'V') camera.cyclePreset();
    if(key == 'c' || key == 'C') camera.toggleLock();
}

void Game::onMouseDrag(float deltaX, float deltaY) {
    if (state != GAME_STATE_PLAYING) return;
    camera.processMouseDrag(deltaX, deltaY);
}

void Game::onResize(int w, int h) {
    windowWidth = w;
    windowHeight = h;
}