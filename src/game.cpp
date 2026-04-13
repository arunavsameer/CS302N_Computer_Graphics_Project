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
int safePathColumn = 0;
Game::Game(int width, int height)
    : windowWidth(width), windowHeight(height),
      state(GAME_STATE_START_SCREEN),
      currentGenerationZ(5.0f * Config::CELL_SIZE),
      cameraTrackZ(0.0f), score(0), startZ(0.0f), coinScore(0),
      deathPosition(0.0f), hasWaterDeath(false)
{
    srand(static_cast<unsigned>(time(nullptr)));
}

void Game::resetGame() {
    state         = GAME_STATE_START_SCREEN;
    eggClicks     = 0;
    coinScore     = 0;
    score         = 0;
    hasWaterDeath = false;
    deathPosition = glm::vec3(0.0f);

    player.reset();
    camera.resetToDefault();

    lanes.clear();
    currentGenerationZ = 5.0f * Config::CELL_SIZE;

    for (int i = 0; i < Config::INITIAL_SAFE_ZONE_LENGTH; i++) {
        lanes.push_back(Lane(currentGenerationZ, LANE_GRASS,safePathColumn));
        currentGenerationZ -= Config::CELL_SIZE;
    }
    for (int i = 0; i < 5; i++) generateLaneBlock();

    cameraTrackZ = player.getPosition().z;
    startZ       = player.getPosition().z;
}

void Game::initialize() {
    renderer.initialize();

    for (int i = 0; i < Config::INITIAL_SAFE_ZONE_LENGTH; i++) {
        lanes.push_back(Lane(currentGenerationZ, LANE_GRASS,safePathColumn));
        currentGenerationZ -= Config::CELL_SIZE;
    }
    for (int i = 0; i < 5; i++) generateLaneBlock();

    cameraTrackZ = player.getPosition().z;
    startZ       = player.getPosition().z;
}

void Game::generateLaneBlock() {
    int r = rand() % 100;
    LaneType nextType;
    int blockWidth = 1;

    if (r < 30) {
        nextType   = LANE_GRASS;
        blockWidth = Config::MIN_GRASS_WIDTH + rand() % ((Config::MAX_GRASS_WIDTH - Config::MIN_GRASS_WIDTH) + 1);
    } else if (r < 60) {
        nextType   = LANE_ROAD;
        blockWidth = Config::MIN_ROAD_WIDTH + rand() % ((Config::MAX_ROAD_WIDTH - Config::MIN_ROAD_WIDTH) + 1);
    } else if (r < 80) {
        nextType   = LANE_RIVER;
        blockWidth = Config::MIN_RIVER_WIDTH + rand() % ((Config::MAX_RIVER_WIDTH - Config::MIN_RIVER_WIDTH) + 1);
    } else {
        nextType   = LANE_RAIL;
        blockWidth = Config::MIN_RAIL_WIDTH + rand() % ((Config::MAX_RAIL_WIDTH - Config::MIN_RAIL_WIDTH) + 1);
    }

    for (int i = 0; i < blockWidth; i++) {
        // smooth path movement
        int shift = rand() % 3 - 1; // -1, 0, +1
        safePathColumn += shift;

        // clamp so it doesn’t go out of bounds
        if (safePathColumn < -5) safePathColumn = -5;
        if (safePathColumn > 5)  safePathColumn = 5;

        LaneType actualType = nextType;
        if (nextType == LANE_RIVER) {
            // Check if the immediately preceding lane was a lilypad lane
            bool prevIsLilypad = (!lanes.empty() && lanes.back().getType() == LANE_LILYPAD);
            
            // If it wasn't, we have a 40% chance to make this river lane a lilypad lane
            if (!prevIsLilypad && (rand() % 100 < 40)) {
                actualType = LANE_LILYPAD;
            }
        }

        lanes.push_back(Lane(currentGenerationZ, actualType, safePathColumn));
        currentGenerationZ -= Config::CELL_SIZE;
    }
}

void Game::updateCameraAndFailState(float deltaTime) {
    glm::vec3 playerBasePos = player.getBasePosition();

    float idealCameraTrackZ = cameraTrackZ - Config::CAMERA_AUTO_SCROLL_SPEED * deltaTime;
    if (playerBasePos.z < idealCameraTrackZ) idealCameraTrackZ = playerBasePos.z;
    cameraTrackZ = idealCameraTrackZ;

    float lerpFactorXY = 1.0f - std::exp(-Config::CAMERA_SMOOTH_SPEED_XY * deltaTime);
    float lerpFactorZ  = 1.0f - std::exp(-Config::CAMERA_SMOOTH_SPEED_Z  * deltaTime);

    smoothedCameraTarget.x = glm::mix(smoothedCameraTarget.x, playerBasePos.x, lerpFactorXY);
    smoothedCameraTarget.y = glm::mix(smoothedCameraTarget.y, playerBasePos.y, lerpFactorXY);
    smoothedCameraTarget.z = glm::mix(smoothedCameraTarget.z, cameraTrackZ,    lerpFactorZ);

    // Pass tflex item gap 6he smoothed target to the actual camera matrix calculations
    camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);

    if (player.getPosition().z > cameraTrackZ + Config::CAMERA_BACKWARD_DEATH_DISTANCE) {
        // Trigger squish animation before switching to game-over state
        if (!player.getIsDead()) {
            player.setDead(true);
            deathPosition = player.getPosition();
            hasWaterDeath = false;
        }
        state = GAME_STATE_GAME_OVER;
    }
}

void Game::maintainInfiniteLanes() {
    glm::vec3 playerPos      = player.getPosition();
    float forwardReferenceZ  = std::min(playerPos.z, cameraTrackZ);

    while (currentGenerationZ > forwardReferenceZ - Config::LANE_GENERATION_BUFFER_AHEAD) {
        generateLaneBlock();
    }

    float pruneBehindZ = playerPos.z + Config::LANE_CLEANUP_BUFFER_BEHIND;
    int removeCount = 0;
    while (removeCount < (int)lanes.size() &&
           lanes[removeCount].getZPosition() > pruneBehindZ) {
        removeCount++;
    }
    if (removeCount > 0)
        lanes.erase(lanes.begin(), lanes.begin() + removeCount);
}

void Game::update(float deltaTime) {
    // Always update lanes (cars/logs animate regardless of game state)
    for (auto& lane : lanes) lane.update(deltaTime);

    if (state == GAME_STATE_START_SCREEN) {
        smoothedCameraTarget = player.getBasePosition();
        camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
        return;
    }

    if (state == GAME_STATE_GAME_OVER) {
        // ── Must update player so water-death particles animate ──────────────
        player.update(deltaTime);

        camera.setTargetRadius(UIConfig::DEAD_ZOOM_RADIUS);
        camera.setLerpSpeed(UIConfig::DEAD_ZOOM_SPEED);

        // For water death the chicken may be off-screen (scrolled away before
        // death was triggered).  Use the stored deathPosition so the camera
        // snaps back to where the splash actually happened.
        glm::vec3 trackPos  = hasWaterDeath ? deathPosition : player.getPosition();
        float     snapSpeed = hasWaterDeath ? 0.14f : 0.05f;

        smoothedCameraTarget.x = glm::mix(smoothedCameraTarget.x, trackPos.x, snapSpeed);
        smoothedCameraTarget.z = glm::mix(smoothedCameraTarget.z, trackPos.z, snapSpeed);
        camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
        return;
    }

    // --- NORMAL PLAYING STATE ---
    player.update(deltaTime);
    checkCollisions(deltaTime);

    if (state != GAME_STATE_PLAYING) return;

    updateCameraAndFailState(deltaTime);
    maintainInfiniteLanes();
    score = coinScore;
}

void Game::checkCollisions(float deltaTime) {
    glm::vec3 playerPos  = player.getPosition();
    glm::vec3 playerSize = player.getSize();

    bool  onLog      = false;
    bool onLilypad  = false;
    Lane* currentLane = nullptr;

    for (auto& lane : lanes) {
        if (std::abs(lane.getZPosition() - playerPos.z) < Config::CELL_SIZE / 2.0f) {
            currentLane = &lane;
            break;
        }
    }
    if (!currentLane) return;

    for (auto& obs : currentLane->getObstacles()) {
        if (!obs.getIsActive()) continue;

        bool isColliding = false;
        if (obs.getType() == OBSTACLE_LOG || obs.getType() == OBSTACLE_LILYPAD) {
            isColliding = Collision::checkAABB(
                player.getBasePosition(), playerSize,
                obs.getPosition(), obs.getSize());
        } else {
            isColliding = Collision::checkAABB(
                playerPos, playerSize,
                obs.getPosition(), obs.getSize());
        }

        if (isColliding) {
            if (obs.getType() == OBSTACLE_CAR || obs.getType() == OBSTACLE_TRAIN) {
                // Squish death
                player.setDead(true);
                deathPosition = playerPos;
                hasWaterDeath = false;
                state         = GAME_STATE_GAME_OVER;
                return;
            }
            else if (obs.getType() == OBSTACLE_LOG) {
                onLog = true;
                obs.setSinking(true);
                player.applyLogVelocity(obs.getSpeed(), deltaTime);
            }
            else if (obs.getType() == OBSTACLE_LILYPAD) {
                onLilypad = true;
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
 // ===== 🌳🪨 DECORATION COLLISION

    // ── Water death ──────────────────────────────────────────────────────────
    // The water surface top is at yPos + scale.y/2
    // = -CELL_SIZE*0.2 + CELL_SIZE*0.1 = -CELL_SIZE*0.1
    if ((currentLane->getType() == LANE_RIVER || currentLane->getType() == LANE_LILYPAD) && !player.getIsJumping() && !onLog && !onLilypad) {
        const float waterSurface = -Config::CELL_SIZE * 0.1f;
        player.triggerWaterDeath(waterSurface);
        deathPosition = playerPos;   // lock camera here BEFORE auto-scroll moves it
        hasWaterDeath = true;
        state         = GAME_STATE_GAME_OVER;
    }

}

void Game::render() {
    renderer.prepareFrame();
    camera.apply();

    for (auto& lane : lanes) lane.render(renderer);

    // Egg (start screen) or chicken (all other states)
    if (state == GAME_STATE_START_SCREEN) {
        glm::vec3 pos = player.getPosition();
        float wobble  = (eggClicks > 0) ?
            std::sin(glutGet(GLUT_ELAPSED_TIME) * 0.01f) * 0.1f * eggClicks : 0.0f;
        float scale   = 0.6f + eggClicks * 0.1f;

        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        glRotatef(wobble * 50.0f, 0, 0, 1);
        renderer.drawCube(glm::vec3(0, 0.4f, 0),
                          glm::vec3(scale, scale * 1.2f, scale),
                          glm::vec3(1.0f, 0.95f, 0.9f));
        glPopMatrix();
    } else {
        player.render(renderer);
    }

    camera.renderOverlay(windowWidth, windowHeight);
    renderUIOverlay();
}

void Game::onKeyPress(unsigned char key) {
    if (state != GAME_STATE_PLAYING) return;

    float dx = 0.0f, dz = 0.0f;

    if (key == 'w' || key == 'W') dz = -1.0f;
    if (key == 's' || key == 'S') dz =  1.0f;
    if (key == 'a' || key == 'A') dx = -1.0f;
    if (key == 'd' || key == 'D') dx =  1.0f;

    if (dx == 0.0f && dz == 0.0f) {
        if (key == 'v' || key == 'V') camera.cyclePreset();
        if (key == 'c' || key == 'C') camera.toggleLock();
        return;
    }

    // ===== PREDICT NEXT POSITION =====
    glm::vec3 currentPos = player.getPosition();
    glm::vec3 nextPos = currentPos + glm::vec3(
        dx * Config::CELL_SIZE,
        0.0f,
        dz * Config::CELL_SIZE
    );

    // ===== FIND TARGET LANE =====
    Lane* targetLane = nullptr;
    for (auto& lane : lanes) {
        if (std::abs(lane.getZPosition() - nextPos.z) < Config::CELL_SIZE / 2.0f) {
            targetLane = &lane;
            break;
        }
    }

    bool blocked = false;

    // ===== CHECK TREE/ROCK COLLISION =====
    if (targetLane && targetLane->getType() == LANE_GRASS) {

        for (auto& d : targetLane->decorations) {

            float size = (d.type == 0) ? 0.6f : 0.5f;

            if (fabs(nextPos.x - d.position.x) < size &&
                fabs(nextPos.z - d.position.z) < size) {

                blocked = true;
                break;
            }
        }
    }

    if (!blocked) {
        player.move(dx, dz);
    }

    // camera controls
    if (key == 'v' || key == 'V') camera.cyclePreset();
    if (key == 'c' || key == 'C') camera.toggleLock();
}

void Game::onMouseDrag(float deltaX, float deltaY) {
    if (state != GAME_STATE_PLAYING) return;
    camera.processMouseDrag(deltaX, deltaY);
}

void Game::onResize(int w, int h) {
    windowWidth  = w;
    windowHeight = h;
}

void Game::renderUIOverlay() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- Score ---
    if (state == GAME_STATE_PLAYING || state == GAME_STATE_GAME_OVER) {
        glColor3f(1.0f, 1.0f, 1.0f);
        std::stringstream ss;
        ss << score;
        std::string scoreStr = ss.str();
        glRasterPos2f(20, windowHeight - 40);
        for (char c : scoreStr) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // --- State-specific UI ---
    if (state == GAME_STATE_START_SCREEN) {
        glColor3f(1.0f, 1.0f, 1.0f);
        std::string msg = "Click the Egg to Hatch!";
        glRasterPos2f(windowWidth / 2.0f - 80.0f, windowHeight / 2.0f + 100.0f);
        for (char c : msg) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    else if (state == GAME_STATE_GAME_OVER) {
        float boxW = 140.0f, boxH = 60.0f;
        float cx   = windowWidth  / 2.0f;
        float cy   = 80.0f;

        // Orange button
        glColor3f(1.0f, 0.6f, 0.4f);
        glBegin(GL_QUADS);
            glVertex2f(cx - boxW/2, cy - boxH/2);
            glVertex2f(cx + boxW/2, cy - boxH/2);
            glVertex2f(cx + boxW/2, cy + boxH/2);
            glVertex2f(cx - boxW/2, cy + boxH/2);
        glEnd();

        // Teal outline
        glLineWidth(4.0f);
        glColor3f(0.1f, 0.4f, 0.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(cx - boxW/2, cy - boxH/2);
            glVertex2f(cx + boxW/2, cy - boxH/2);
            glVertex2f(cx + boxW/2, cy + boxH/2);
            glVertex2f(cx - boxW/2, cy + boxH/2);
        glEnd();
        glLineWidth(1.0f);

        // Play triangle
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
            glVertex2f(cx - 15, cy - 20);
            glVertex2f(cx - 15, cy + 20);
            glVertex2f(cx + 25, cy);
        glEnd();
    }

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Game::onMouseClick(int button, int clickState, int x, int y) {
    if (clickState != 0) return;

    if (state == GAME_STATE_START_SCREEN) {
        eggClicks++;
        if (eggClicks >= UIConfig::MAX_EGG_CLICKS) {
            state = GAME_STATE_PLAYING;
            player.move(0.0f, 0.0f);
        }
    }
    else if (state == GAME_STATE_GAME_OVER) {
        int   invertedY = windowHeight - y;
        float boxW = 140.0f, boxH = 60.0f;
        float cx   = windowWidth  / 2.0f;
        float cy   = 80.0f;

        if (x > cx - boxW/2 && x < cx + boxW/2 &&
            invertedY > cy - boxH/2 && invertedY < cy + boxH/2) {
            resetGame();
        }
    }
}