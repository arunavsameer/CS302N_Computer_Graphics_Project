#include "../include/game.h"
#include "../include/collision.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <sstream>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#include "../include/coin.h"
Game::Game(int width, int height)
    : windowWidth(width),
      windowHeight(height),
      state(GAME_STATE_PLAYING),
      currentGenerationZ(5.0f * Config::CELL_SIZE),
      cameraTrackZ(0.0f) {
    srand(static_cast<unsigned>(time(nullptr)));
    score = 0;
startZ = 0.0f;
}

void Game::initialize() {
    renderer.initialize();

    // Build the massive starting safe zone
    for (int i = 0; i < Config::INITIAL_SAFE_ZONE_LENGTH; i++) {
        lanes.push_back(Lane(currentGenerationZ, LANE_GRASS));
        currentGenerationZ -= Config::CELL_SIZE;
    }

    for (int i = 0; i < 5; i++) {
        generateLaneBlock();
    }

    // Keep the camera anchor aligned with the start position
    cameraTrackZ = player.getPosition().z;
    startZ = player.getPosition().z;
}

void Game::generateLaneBlock() {
    int r = rand() % 100;
    LaneType nextType;
    int blockWidth = 1;

    if (r < 30) {
        nextType = LANE_GRASS;
        blockWidth = Config::MIN_GRASS_WIDTH + rand() % ((Config::MAX_GRASS_WIDTH - Config::MIN_GRASS_WIDTH) + 1);
    } else if (r < 60) {
        nextType = LANE_ROAD;
        blockWidth = Config::MIN_ROAD_WIDTH + rand() % ((Config::MAX_ROAD_WIDTH - Config::MIN_ROAD_WIDTH) + 1);
    } else if (r < 80) {
        nextType = LANE_RIVER;
        blockWidth = Config::MIN_RIVER_WIDTH + rand() % ((Config::MAX_RIVER_WIDTH - Config::MIN_RIVER_WIDTH) + 1);
    } else {
        nextType = LANE_RAIL;
        blockWidth = Config::MIN_RAIL_WIDTH + rand() % ((Config::MAX_RAIL_WIDTH - Config::MIN_RAIL_WIDTH) + 1);
    }

    for (int i = 0; i < blockWidth; i++) {
        lanes.push_back(Lane(currentGenerationZ, nextType));
        currentGenerationZ -= Config::CELL_SIZE;
    }
}

void Game::updateCameraAndFailState(float deltaTime) {
    glm::vec3 playerBasePos = player.getBasePosition();

    // Camera anchor always moves forward (negative Z is forward in this project).
    // If the player goes further forward, the camera keeps up; if they stall, the camera eventually overtakes them.
    if (playerBasePos.z < cameraTrackZ) {
        cameraTrackZ = playerBasePos.z;
    }
    cameraTrackZ -= Config::CAMERA_AUTO_SCROLL_SPEED * deltaTime;

    glm::vec3 cameraTarget = playerBasePos;
    cameraTarget.z = cameraTrackZ;

    camera.update(deltaTime, windowWidth, windowHeight, cameraTarget);

    // If the chicken falls too far behind the moving camera target, the run is over.
    if (player.getPosition().z > cameraTrackZ + Config::CAMERA_BACKWARD_DEATH_DISTANCE) {
        state = GAME_STATE_GAME_OVER;
    }
}

void Game::maintainInfiniteLanes() {
    glm::vec3 playerPos = player.getPosition();
    float forwardReferenceZ = std::min(playerPos.z, cameraTrackZ);

    // Keep generating until there is enough world in front of the camera/player.
    while (currentGenerationZ > forwardReferenceZ - Config::LANE_GENERATION_BUFFER_AHEAD) {
        generateLaneBlock();
    }

    // Drop lanes that are well behind the player so memory stays bounded.
    float pruneBehindZ = playerPos.z + Config::LANE_CLEANUP_BUFFER_BEHIND;
    while (!lanes.empty() && lanes.front().getZPosition() > pruneBehindZ) {
        lanes.erase(lanes.begin());
    }
}

void Game::update(float deltaTime) {
    if (state != GAME_STATE_PLAYING) return;

    player.update(deltaTime);

    for (auto& lane : lanes) {
        lane.update(deltaTime);
    }

    checkCollisions(deltaTime);

    if (state != GAME_STATE_PLAYING) return;

    updateCameraAndFailState(deltaTime);
    if (state != GAME_STATE_PLAYING) return;

    maintainInfiniteLanes();

    score = coinScore;
}

void Game::checkCollisions(float deltaTime) {
    glm::vec3 playerPos = player.getPosition();
    glm::vec3 playerSize = player.getSize();

    bool onLog = false;
    Lane* currentLane = nullptr;

    for (auto& lane : lanes) {
        if (std::abs(lane.getZPosition() - playerPos.z) < Config::CELL_SIZE / 2.0f) {
            currentLane = &lane;
            break;
        }
    }

    if (!currentLane) return; 

    for (const auto& obs : currentLane->getObstacles()) {
        if (!obs.getIsActive()) continue; // Ignore trains that have already passed by

        if (Collision::checkAABB(playerPos, playerSize, obs.getPosition(), obs.getSize())) {

            if (obs.getType() == OBSTACLE_CAR || obs.getType() == OBSTACLE_TRAIN) {
                state = GAME_STATE_GAME_OVER; 
                return;
            } 
            else if (obs.getType() == OBSTACLE_LOG) {
                onLog = true;
                player.applyLogVelocity(obs.getSpeed(), deltaTime);
            }
        }
    }
     // ===== COIN COLLISION =====
for (auto& coin : currentLane->coins) {

    if (coin.collected) continue;

    glm::vec3 coinPos = coin.getPosition();
    float coinSize = coin.getSize();

    if (fabs(playerPos.x - coinPos.x) < playerSize.x &&
        fabs(playerPos.z - coinPos.z) < playerSize.z) {

        coin.collected = true;

        coinScore += 10;   // 🔥 ADD COIN SCORE
    }
}

    if (currentLane->getType() == LANE_RIVER && !player.getIsJumping() && !onLog) {
        state = GAME_STATE_GAME_OVER; 
    }
}

void Game::render() {
    renderer.prepareFrame();
    camera.apply();

    for (auto& lane : lanes) {
        lane.render(renderer);
    }
    player.render(renderer);

    camera.renderOverlay(windowWidth, windowHeight);

    // ===== SCORE DISPLAY (INSIDE FUNCTION) =====
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);

    std::stringstream ss;
    ss << "Score: " << score;
    std::string text = ss.str();

    glRasterPos2f(20, windowHeight - 30);

    for (char c : text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    // restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Game::onKeyPress(unsigned char key) {
    if (state != GAME_STATE_PLAYING) return;

    if (key == 'w' || key == 'W') player.move(0.0f, -1.0f);
    if (key == 's' || key == 'S') player.move(0.0f, 1.0f);
    if (key == 'a' || key == 'A') player.move(-1.0f, 0.0f);
    if (key == 'd' || key == 'D') player.move(1.0f, 0.0f);

    if (key == 'v' || key == 'V') camera.cyclePreset();
    if (key == 'c' || key == 'C') camera.toggleLock();
}

void Game::onMouseDrag(float deltaX, float deltaY) {
    if (state != GAME_STATE_PLAYING) return;
    camera.processMouseDrag(deltaX, deltaY);
}

void Game::onResize(int w, int h) {
    windowWidth = w;
    windowHeight = h;
}

