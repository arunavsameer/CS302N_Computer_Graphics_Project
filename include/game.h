#ifndef GAME_H
#define GAME_H

#include "types.h"
#include <glm/glm.hpp>

class Game {
private:
    int windowWidth;
    int windowHeight;
    GameState gameState;
    GameData gameData;
    float deltaTime;
    float lastTime;

public:
    Game(int width, int height);
    ~Game();
    
    // Core game functions
    bool initialize();
    void update();
    void render();
    void cleanup();
    
    // Event handlers
    void onKeyPress(unsigned char key);
    void onKeyRelease(unsigned char key);
    void onSpecialKeyPress(int key);
    void onSpecialKeyRelease(int key);
    void onMouseClick(int button, int state, float x, float y);
    void onMouseMove(float x, float y);
    
    // Getters
    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }
    GameState getGameState() const { return gameState; }
    GameData getGameData() const { return gameData; }
    bool isRunning() const { return gameState != GAME_STATE_GAME_OVER; }
};

#endif