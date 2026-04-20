#ifndef PREGAME_H
#define PREGAME_H

#include "types.h"
#include "renderer.h"
#include "character.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

class PreGameManager {
public:
    PreGameManager(int windowWidth, int windowHeight);
    
    // Update pre-game state
    void update(float deltaTime, GameState& state, Chicken& player, int& eggClicks, 
                int& lastClickTime, int& selectedCharacterIndex);
    
    // Render UI for pre-game states
    void render(GameState state, const Chicken& player, int windowWidth, int windowHeight,
                int eggClicks, int lastClickTime, int selectedCharacterIndex,
                int highScore, int totalCoins, int score, int coinScore);
    
    // Handle keyboard input
    void onKeyPress(unsigned char key, GameState& state, int& selectedCharacterIndex,
                    Chicken& player, int numChars = 5);
    
    // Handle special keys (arrow keys)
    void onSpecialKey(int key, GameState& state, int& selectedCharacterIndex, int numChars = 5);
    
    // Handle mouse clicks
    void onMouseClick(int button, int clickState, int x, int y, GameState& state,
                      int& selectedCharacterIndex, Chicken& player, int windowWidth,
                      int windowHeight, int& eggClicks, int& lastClickTime);
    
private:
    int windowWidth, windowHeight;
    
    // UI Rendering helpers
    void renderMainMenu(int windowWidth, int windowHeight, int highScore, int totalCoins);
    void renderCharacterSelect(int windowWidth, int windowHeight, int selectedCharacterIndex,
                               int timeMs);
    void renderStartScreen(int windowWidth, int windowHeight, int timeMs);
    void renderGameOver(int windowWidth, int windowHeight, int score, int coinScore,
                        int highScore, int timeMs);
    
    // Helper functions for rendering
    void drawCharacterIcon(float icx, float icy, float sz, int ci, bool selected, int timeMs);
};

#endif
