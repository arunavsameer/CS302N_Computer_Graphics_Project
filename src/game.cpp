#include "../include/game.h"
#include "../include/collision.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

Game::Game(int width, int height) : windowWidth(width), windowHeight(height), state(GAME_STATE_PLAYING), currentGenerationZ(5.0f * Config::CELL_SIZE) {
    srand(static_cast<unsigned>(time(nullptr))); 
}

void Game::initialize() {
    renderer.initialize();
    
    // Initial safe zone - Spawns a long runway of grass behind and ahead of the chicken
    for (int i = 0; i < Config::INITIAL_SAFE_ZONE_LENGTH; i++) {
        lanes.push_back(Lane(currentGenerationZ, LANE_GRASS));
        currentGenerationZ -= Config::CELL_SIZE;
    }

    // Begin generating hazard biomes
    for(int i = 0; i < 5; i++) {
        generateLaneBlock();
    }
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

void Game::update(float deltaTime) {
    if (state != GAME_STATE_PLAYING) return;

    player.update(deltaTime);
    
    for (auto& lane : lanes) {
        lane.update(deltaTime);
    }

    checkCollisions(deltaTime);

    camera.update(deltaTime, windowWidth, windowHeight, player.getBasePosition());
    
    // Endless Generation
    if (player.getPosition().z - currentGenerationZ < 20.0f * Config::CELL_SIZE) {
        generateLaneBlock();
    }
}

void Game::checkCollisions(float deltaTime) {
    glm::vec3 playerPos = player.getPosition();
    glm::vec3 playerSize = player.getSize(); // The exact AABB bounds of the chicken
    
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
        if (!obs.getIsActive()) continue; // Ignore trains that have already passed

        // Bounding Box check - now guaranteed to be cuboids matching the visuals
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

    // Water Logic check
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
}

void Game::onKeyPress(unsigned char key) {
    if (state != GAME_STATE_PLAYING) return;

    if(key == 'w' || key == 'W') player.move(0.0f, -1.0f);
    if(key == 's' || key == 'S') player.move(0.0f, 1.0f);
    if(key == 'a' || key == 'A') player.move(-1.0f, 0.0f);
    if(key == 'd' || key == 'D') player.move(1.0f, 0.0f);
    
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