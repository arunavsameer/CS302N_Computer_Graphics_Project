#ifndef GAME_H
#define GAME_H

#include <vector>
#include "types.h"
#include "renderer.h"
#include "camera.h"
#include "character.h"
#include "lane.h"
#include "pregame.h"

class Game {
private:
    int windowWidth, windowHeight;
    GameState state;
    Renderer renderer;
    Camera camera;
    Chicken player;
    PreGameManager preGameManager;
    std::vector<Lane> lanes;
    float currentGenerationZ;
    float cameraTrackZ;
    int score;
    int highScore; 
    float startZ;
    int coinScore = 0;
    int totalCoins = 0; // <--- ADD THIS HERE

    // ── Death tracking ───────────────────────────────────────────────────────
    glm::vec3 deathPosition;
    bool      hasWaterDeath;

    // ── Fast-stream death flag ────────────────────────────────────────────────
    // Set true when the chicken is swept off the screen by a fast-stream log.
    // The camera will zoom to deathPosition (same as water death).
    bool      hasStreamDeath;

    void generateLaneBlock();
    void checkCollisions(float deltaTime);
    void updateCameraAndFailState(float deltaTime);
    void maintainInfiniteLanes();

    // Renders the mountain walls, waterfalls, foam, and back wall
    // based on the currently loaded lanes and the player's position.
    void renderWorldBoundaries();

    glm::vec3 smoothedCameraTarget;
    int eggClicks    = 0;
    int lastClickTime = 0;
    bool nightMode   = false;   // toggled with N key
    int selectedCharacterIndex = 0; // index into the character carousel (0–4)
    
    // ── Day/Night Cycle & Shadows ────────────────────────────────────────────
    float currentGameTime = 30.0f;  // Game time in seconds (starts at midday: cycleTime ~0.3)
    float sunAngle = 0.0f;         // Current angle of the sun (radians)

    void updateDayNightCycle(float deltaTime);
    void renderShadows();
    void resetGame();
    void renderUIOverlay();

public:
    Game(int width, int height);
    void initialize();
    void update(float deltaTime);
    void render();
    void onKeyPress(unsigned char key);
    void onSpecialKey(int key);   // handles arrow keys (GLUT_KEY_LEFT / RIGHT)
    void onResize(int w, int h);
    void onMouseDrag(float deltaX, float deltaY);
    void onMouseClick(int button, int state, int x, int y);

    bool getjumpstatus() const { return player.getIsJumping(); }
};

#endif