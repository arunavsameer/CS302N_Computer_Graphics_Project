#include "../include/game.h"
#include <iostream>

Game::Game(int width, int height) 
    : windowWidth(width), windowHeight(height), deltaTime(0.0f), lastTime(0.0f) {
    gameState = GAME_STATE_PLAYING;
    gameData.score = 0;
    gameData.coins = 0;
    gameData.health = 100;
    gameData.level = 1;
    gameData.playerPos = glm::vec3(0, 0, 0);
    gameData.isGameOver = false;
}

Game::~Game() {
    cleanup();
}

bool Game::initialize() {
    std::cout << "[Game] Initializing..." << std::endl;
    std::cout << "[Game] Window: " << windowWidth << "x" << windowHeight << std::endl;
    std::cout << "[Game] Ready!" << std::endl;
    return true;
}

void Game::update() {
    // Update game state
    // TODO: Update chicken position
    // TODO: Update obstacles
    // TODO: Check collisions
}

void Game::render() {
    // Render game
    // TODO: Draw background
    // TODO: Draw lanes
    // TODO: Draw chicken
    // TODO: Draw obstacles
}

void Game::cleanup() {
    std::cout << "[Game] Cleaning up..." << std::endl;
}

void Game::onKeyPress(unsigned char key) {
    std::cout << "[Game] Key pressed: " << key << std::endl;
}

void Game::onKeyRelease(unsigned char key) {
    std::cout << "[Game] Key released: " << key << std::endl;
}

void Game::onSpecialKeyPress(int key) {
    std::cout << "[Game] Special key pressed: " << key << std::endl;
}

void Game::onSpecialKeyRelease(int key) {
    std::cout << "[Game] Special key released: " << key << std::endl;
}

void Game::onMouseClick(int button, int state, float x, float y) {
    std::cout << "[Game] Mouse click: " << button << " at " << x << ", " << y << std::endl;
}

void Game::onMouseMove(float x, float y) {
    // std::cout << "[Game] Mouse move: " << x << ", " << y << std::endl;
}