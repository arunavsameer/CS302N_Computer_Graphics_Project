#include "game.h"

Game::Game(int width, int height) : windowWidth(width), windowHeight(height), state(GAME_STATE_PLAYING) {}

void Game::initialize() {
    renderer.initialize();
    
    // Generate starter lanes, spaced exactly 1 CELL_SIZE apart
    for (int i = 5; i >= -15; i--) {
        LaneType type = (i % 3 == 0) ? LANE_ROAD : LANE_GRASS;
        
        // Multiply the loop index by CELL_SIZE to get the exact Z position
        lanes.push_back(Lane((float)i * CELL_SIZE, type)); 
    }
}

void Game::update(float deltaTime) {
    if (state != GAME_STATE_PLAYING) return;

    player.update(deltaTime);
    camera.update(windowWidth, windowHeight, player.getPosition());
    
    for (auto& lane : lanes) {
        lane.update(deltaTime);
    }
    
    // Collision calls would go here using the Collision class
}

void Game::render() {
    renderer.prepareFrame();
    camera.apply();

    for (auto& lane : lanes) {
        lane.render(renderer);
    }
    player.render(renderer);
}

void Game::onKeyPress(unsigned char key) {
    if (state != GAME_STATE_PLAYING) return;

    if(key == 'w' || key == 'W') player.move(0.0f, -1.0f);
    if(key == 's' || key == 'S') player.move(0.0f, 1.0f);
    if(key == 'a' || key == 'A') player.move(-1.0f, 0.0f);
    if(key == 'd' || key == 'D') player.move(1.0f, 0.0f);
}

void Game::onResize(int w, int h) {
    windowWidth = w;
    windowHeight = h;
}