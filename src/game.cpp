#include "../include/game.h"
#include "../include/collision.h"
#include "../include/types.h"
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
    : windowWidth(width), windowHeight(height),
      state(GAME_STATE_START_SCREEN), // START HERE INSTEAD OF PLAYING
      currentGenerationZ(5.0f * Config::CELL_SIZE),
      cameraTrackZ(0.0f), score(0), startZ(0.0f), coinScore(0) {
    srand(static_cast<unsigned>(time(nullptr)));
}

void Game::resetGame() {
    state = GAME_STATE_START_SCREEN;
    eggClicks = 0;
    coinScore = 0;
    score = 0;
    
    player.reset();
    camera.resetToDefault();
    
    lanes.clear();
    currentGenerationZ = 5.0f * Config::CELL_SIZE;
    
    // Rebuild initial zone
    for (int i = 0; i < Config::INITIAL_SAFE_ZONE_LENGTH; i++) {
        lanes.push_back(Lane(currentGenerationZ, LANE_GRASS));
        currentGenerationZ -= Config::CELL_SIZE;
    }
    for (int i = 0; i < 5; i++) generateLaneBlock();
    
    cameraTrackZ = player.getPosition().z;
    startZ = player.getPosition().z;
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

    // Auto-scroll the logical track forward
    float idealCameraTrackZ = cameraTrackZ - Config::CAMERA_AUTO_SCROLL_SPEED * deltaTime;
    
    // If the player pushes further ahead, snap the logical track instantly
    if (playerBasePos.z < idealCameraTrackZ) {
        idealCameraTrackZ = playerBasePos.z;
    }
    cameraTrackZ = idealCameraTrackZ;

    // Calculate framerate-independent lerp factors
    float lerpFactorXY = 1.0f - std::exp(-Config::CAMERA_SMOOTH_SPEED_XY * deltaTime);
    float lerpFactorZ  = 1.0f - std::exp(-Config::CAMERA_SMOOTH_SPEED_Z * deltaTime);

    // Smoothly interpolate X/Y to follow the player, and Z to follow the logical track
    smoothedCameraTarget.x = glm::mix(smoothedCameraTarget.x, playerBasePos.x, lerpFactorXY);
    smoothedCameraTarget.y = glm::mix(smoothedCameraTarget.y, playerBasePos.y, lerpFactorXY);
    smoothedCameraTarget.z = glm::mix(smoothedCameraTarget.z, cameraTrackZ,  lerpFactorZ);

    // Pass the smoothed target to the actual camera matrix calculations
    camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);

    // Use the strict logical track (cameraTrackZ) for the death condition, 
    // ensuring the game mechanics remain perfectly fair and responsive.
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
    int removeCount = 0;
    while (removeCount < lanes.size() && lanes[removeCount].getZPosition() > pruneBehindZ) {
        removeCount++;
    }
    
    if (removeCount > 0) {
        lanes.erase(lanes.begin(), lanes.begin() + removeCount);
    }
}

void Game::update(float deltaTime) {
    // 1. Always update lanes so cars/logs move even when dead or in start screen
    for (auto& lane : lanes) {
        lane.update(deltaTime);
    }

    if (state == GAME_STATE_START_SCREEN) {
        // Idle camera around the egg
        smoothedCameraTarget = player.getBasePosition();
        camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
        return; 
    }

    if (state == GAME_STATE_GAME_OVER) {
        // Dramatic Zoom on the dead chicken
        camera.setTargetRadius(UIConfig::DEAD_ZOOM_RADIUS);
        camera.setLerpSpeed(UIConfig::DEAD_ZOOM_SPEED);
        
        // Lock camera purely to the player body
        smoothedCameraTarget.x = glm::mix(smoothedCameraTarget.x, player.getPosition().x, 0.05f);
        smoothedCameraTarget.z = glm::mix(smoothedCameraTarget.z, player.getPosition().z, 0.05f);
        camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
        return;
    }

    // --- NORMAL PLAYING STATE BELOW ---
    player.update(deltaTime);
    checkCollisions(deltaTime);
    
    if (state != GAME_STATE_PLAYING) return; // Catch death from collisions

    updateCameraAndFailState(deltaTime);
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

        bool isColliding = false;
        if (obs.getType() == OBSTACLE_LOG) {
            isColliding = Collision::checkAABB(player.getBasePosition(), playerSize, obs.getPosition(), obs.getSize());
        } else {
            isColliding = Collision::checkAABB(playerPos, playerSize, obs.getPosition(), obs.getSize());
        }

        if (isColliding) {
            if (obs.getType() == OBSTACLE_CAR || obs.getType() == OBSTACLE_TRAIN) {
                player.setDead(true); // Tell chicken to squish
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

        coinScore += 10;
    }
}

    if (currentLane->getType() == LANE_RIVER && !player.getIsJumping() && !onLog) {
        player.setDead(true);
        state = GAME_STATE_GAME_OVER; 
    }
}

void Game::render() {
    renderer.prepareFrame();
    camera.apply();

    for (auto& lane : lanes) {
        lane.render(renderer);
    }

    // Render Egg OR Chicken
    if (state == GAME_STATE_START_SCREEN) {
        glm::vec3 pos = player.getPosition();
        // Calculate a wobble effect based on clicks
        float wobble = (eggClicks > 0) ? sin(glutGet(GLUT_ELAPSED_TIME) * 0.01f) * 0.1f * eggClicks : 0.0f;
        float scale = 0.6f + (eggClicks * 0.1f); // Grow slightly when clicked
        
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        glRotatef(wobble * 50.0f, 0, 0, 1);
        renderer.drawCube(glm::vec3(0,0,0), glm::vec3(scale, scale * 1.2f, scale), glm::vec3(1.0f, 0.95f, 0.9f));
        glPopMatrix();
    } else {
        player.render(renderer);
    }

    camera.renderOverlay(windowWidth, windowHeight);
    renderUIOverlay();
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

void Game::renderUIOverlay() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Y=0 is at the bottom, Y=windowHeight is at the top
    gluOrtho2D(0, windowWidth, 0, windowHeight); 
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- UNIVERSAL SCORE DISPLAY ---
    if (state == GAME_STATE_PLAYING || state == GAME_STATE_GAME_OVER) {
        // Draw Main Score (Top Left)
        glColor3f(1.0f, 1.0f, 1.0f);
        std::stringstream ss; 
        ss << score;
        std::string scoreStr = ss.str();
        
        // Use a larger font for the score if available, or scale it
        // We'll use HELVETICA_18 and draw it near the top left
        glRasterPos2f(20, windowHeight - 40);
        for (char c : scoreStr) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }

        // Draw Coin Score (Top Right)
        // std::stringstream css; 
        // css << coinScore << " c";
        // std::string coinStr = css.str();
        
        // glColor3f(1.0f, 0.8f, 0.0f); // Yellowish for coins
        // // Approximate width offset
        // glRasterPos2f(windowWidth - 80, windowHeight - 40); 
        // for (char c : coinStr) {
        //     glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        // }
    }

    // --- STATE SPECIFIC UI ---
    if (state == GAME_STATE_START_SCREEN) {
        glColor3f(1.0f, 1.0f, 1.0f);
        std::string msg = "Click the Egg to Hatch!";
        glRasterPos2f(windowWidth / 2.0f - 80.0f, windowHeight / 2.0f + 100.0f);
        for (char c : msg) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    else if (state == GAME_STATE_GAME_OVER) {
        // NO BLACK OVERLAY - Just draw the button right over the active game

        // Button dimensions and position (Bottom Center)
        float boxW = 140.0f, boxH = 60.0f;
        float cx = windowWidth / 2.0f;
        float cy = 80.0f; // 80 pixels from the bottom

        // 1. Draw Orange Box
        glColor3f(1.0f, 0.6f, 0.4f); // Peach/Orange color matching the image
        glBegin(GL_QUADS);
            glVertex2f(cx - boxW/2, cy - boxH/2);
            glVertex2f(cx + boxW/2, cy - boxH/2);
            glVertex2f(cx + boxW/2, cy + boxH/2);
            glVertex2f(cx - boxW/2, cy + boxH/2);
        glEnd();

        // 2. Draw Dark Teal Outline (Optional, adds pop like the reference)
        glLineWidth(4.0f);
        glColor3f(0.1f, 0.4f, 0.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(cx - boxW/2, cy - boxH/2);
            glVertex2f(cx + boxW/2, cy - boxH/2);
            glVertex2f(cx + boxW/2, cy + boxH/2);
            glVertex2f(cx - boxW/2, cy + boxH/2);
        glEnd();
        glLineWidth(1.0f);

        // 3. Draw White Play Triangle
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
            glVertex2f(cx - 15, cy - 20); // Bottom-left of triangle
            glVertex2f(cx - 15, cy + 20); // Top-left of triangle
            glVertex2f(cx + 25, cy);      // Right point of triangle
        glEnd();
    } 

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void Game::onMouseClick(int button, int clickState, int x, int y) {
    if (clickState != 0) return; // 0 is usually GLUT_DOWN

    if (state == GAME_STATE_START_SCREEN) {
        eggClicks++;
        if (eggClicks >= UIConfig::MAX_EGG_CLICKS) {
            state = GAME_STATE_PLAYING;
            player.move(0.0f, 0.0f); 
        }
    } 
    else if (state == GAME_STATE_GAME_OVER) {
        // GLUT gives 'y' starting from 0 at the TOP of the window.
        // Our OpenGL UI draws 'y' starting from 0 at the BOTTOM.
        int invertedY = windowHeight - y; 
        
        // New button coordinates matching the render function
        float boxW = 140.0f, boxH = 60.0f;
        float cx = windowWidth / 2.0f;
        float cy = 80.0f; // 80 pixels from the bottom

        // Check if click is inside the orange button
        if (x > cx - boxW/2 && x < cx + boxW/2 && 
            invertedY > cy - boxH/2 && invertedY < cy + boxH/2) {
            resetGame();
        }
    }
}
