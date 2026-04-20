#include "../include/game.h"
#include "../include/collision.h"
#include "../include/types.h"
#include "../include/save_data.h"
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
      state(GAME_STATE_MAIN_MENU), 
      preGameManager(width, height),
      currentGenerationZ(5.0f * Config::CELL_SIZE),
      cameraTrackZ(0.0f), score(0), highScore(0), startZ(0.0f), coinScore(0), totalCoins(0),
      deathPosition(0.0f), hasWaterDeath(false), hasStreamDeath(false),
      lastClickTime(0)
{
    srand(static_cast<unsigned>(time(nullptr)));
    
    // Load save data
    uint64_t loadedCoins = 0;
    uint64_t loadedScore = 0;
    if (SaveManager::loadData(loadedCoins, loadedScore)) {
        totalCoins = loadedCoins;
        highScore = loadedScore;
    } else {
        totalCoins = 0;
        highScore = 0;
    }
}

void Game::initialize()
{
    renderer.initialize();
    resetGame(); // Call reset right away to ensure a perfectly clean setup
}

void Game::resetGame()
{
    // DO NOT reset highScore here!
    eggClicks = 0;
    coinScore = 0;
    score = 0;
    hasWaterDeath = false;
    hasStreamDeath = false;
    deathPosition = glm::vec3(0.0f);
    currentGameTime = 30.0f;  

    player.reset();
    camera.resetToDefault();

    lanes.clear();
    currentGenerationZ = 5.0f * Config::CELL_SIZE;
    safePathColumn = 0;

    for (int i = 0; i < Config::INITIAL_SAFE_ZONE_LENGTH; i++)
    {
        lanes.push_back(Lane(currentGenerationZ, LANE_GRASS, safePathColumn));
        currentGenerationZ -= Config::CELL_SIZE;
    }
    for (int i = 0; i < 5; i++)
        generateLaneBlock();

    cameraTrackZ = player.getPosition().z;
    startZ = player.getPosition().z;

    state = GAME_STATE_MAIN_MENU;
}
void Game::generateLaneBlock()
{
    int r = rand() % 100;
    LaneType nextType;
    int blockWidth = 1;

    if (r < 30)
    {
        nextType = LANE_GRASS;
        blockWidth = Config::MIN_GRASS_WIDTH + rand() % (Config::MAX_GRASS_WIDTH - Config::MIN_GRASS_WIDTH + 1);
    }
    else if (r < 60)
    {
        nextType = LANE_ROAD;
        blockWidth = Config::MIN_ROAD_WIDTH + rand() % (Config::MAX_ROAD_WIDTH - Config::MIN_ROAD_WIDTH + 1);
    }
    else if (r < 80)
    {
        nextType = LANE_RIVER;
        blockWidth = Config::MIN_RIVER_WIDTH + rand() % (Config::MAX_RIVER_WIDTH - Config::MIN_RIVER_WIDTH + 1);
    }
    else
    {
        nextType = LANE_RAIL;
        blockWidth = Config::MIN_RAIL_WIDTH + rand() % (Config::MAX_RAIL_WIDTH - Config::MIN_RAIL_WIDTH + 1);
    }

    if (!lanes.empty())
    {
        LaneType prevBlockType = lanes.back().getType();
        bool prevIsTunnel = (prevBlockType == LANE_ROAD || prevBlockType == LANE_RAIL);
        bool nextIsTunnel = (nextType == LANE_ROAD || nextType == LANE_RAIL);

        if (prevIsTunnel && nextIsTunnel && prevBlockType != nextType)
        {
            int bufWidth = Config::MIN_GRASS_WIDTH + rand() % (Config::MAX_GRASS_WIDTH - Config::MIN_GRASS_WIDTH + 1);
            for (int g = 0; g < bufWidth; g++)
            {
                int shift = rand() % 3 - 1;
                safePathColumn = std::max(-5, std::min(5, safePathColumn + shift));
                lanes.push_back(Lane(currentGenerationZ, LANE_GRASS, safePathColumn));
                currentGenerationZ -= Config::CELL_SIZE;
            }
        }
    }

    for (int i = 0; i < blockWidth; i++)
    {
        int shift = rand() % 3 - 1;
        safePathColumn += shift;
        if (safePathColumn < -5)
            safePathColumn = -5;
        if (safePathColumn > 5)
            safePathColumn = 5;

        LaneType actualType = nextType;
        if (nextType == LANE_RIVER)
        {
            bool prevIsLilypad = (!lanes.empty() && lanes.back().getType() == LANE_LILYPAD);
            if (!prevIsLilypad && (rand() % 100 < 40))
                actualType = LANE_LILYPAD;
        }

        lanes.push_back(Lane(currentGenerationZ, actualType, safePathColumn));
        currentGenerationZ -= Config::CELL_SIZE;
    }
}

void Game::updateCameraAndFailState(float deltaTime)
{
    glm::vec3 playerBasePos = player.getBasePosition();

    float idealCameraTrackZ = cameraTrackZ - Config::CAMERA_AUTO_SCROLL_SPEED * deltaTime;
    if (playerBasePos.z < idealCameraTrackZ)
        idealCameraTrackZ = playerBasePos.z;
    cameraTrackZ = idealCameraTrackZ;

    float lerpXY = 1.0f - std::exp(-Config::CAMERA_SMOOTH_SPEED_XY * deltaTime);
    float lerpZ = 1.0f - std::exp(-Config::CAMERA_SMOOTH_SPEED_Z * deltaTime);

    smoothedCameraTarget.x = glm::mix(smoothedCameraTarget.x, playerBasePos.x, lerpXY);
    smoothedCameraTarget.y = glm::mix(smoothedCameraTarget.y, playerBasePos.y, lerpXY);
    smoothedCameraTarget.z = glm::mix(smoothedCameraTarget.z, cameraTrackZ, lerpZ);

    camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);

    if (player.getPosition().z > cameraTrackZ + Config::CAMERA_BACKWARD_DEATH_DISTANCE)
    {
        if (!player.getIsDead())
        {
            player.setDead(true);
            deathPosition = player.getPosition();
            hasWaterDeath = false;
        }
        state = GAME_STATE_GAME_OVER;
        SaveManager::saveData(totalCoins, highScore);
    }
}

void Game::maintainInfiniteLanes()
{
    glm::vec3 playerPos = player.getPosition();
    float forwardReferenceZ = std::min(playerPos.z, cameraTrackZ);

    while (currentGenerationZ > forwardReferenceZ - Config::LANE_GENERATION_BUFFER_AHEAD)
        generateLaneBlock();

    float pruneBehindZ = playerPos.z + Config::LANE_CLEANUP_BUFFER_BEHIND;
    int removeCount = 0;
    while (removeCount < (int)lanes.size() &&
           lanes[removeCount].getZPosition() > pruneBehindZ)
        removeCount++;
    if (removeCount > 0)
        lanes.erase(lanes.begin(), lanes.begin() + removeCount);
}

void Game::update(float deltaTime)
{
    for (auto &lane : lanes)
        lane.update(deltaTime);

    updateDayNightCycle(deltaTime);

    // Stop game logic from running during menus!
    if (state == GAME_STATE_MAIN_MENU || state == GAME_STATE_CHARACTER_SELECT || state == GAME_STATE_START_SCREEN)
    {
        smoothedCameraTarget = player.getBasePosition();
        camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
        return;
    }

    if (state == GAME_STATE_GAME_OVER)
    {
        player.update(deltaTime);
        camera.setTargetRadius(Config::DEAD_ZOOM_RADIUS);
        camera.setLerpSpeed(Config::DEAD_ZOOM_SPEED);
        bool useWater = hasWaterDeath || hasStreamDeath;
        glm::vec3 trackPos = useWater ? deathPosition : player.getPosition();
        float snapSpeed = useWater ? 0.14f : 0.05f;

        smoothedCameraTarget.x = glm::mix(smoothedCameraTarget.x, trackPos.x, snapSpeed);
        smoothedCameraTarget.z = glm::mix(smoothedCameraTarget.z, trackPos.z, snapSpeed);
        camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
        return;
    }

    player.update(deltaTime);
    checkCollisions(deltaTime);

    if (state != GAME_STATE_PLAYING)
        return;

    updateCameraAndFailState(deltaTime);
    maintainInfiniteLanes();

    // High Score logic properly isolated here
    int lanesMoved = static_cast<int>(std::round((startZ - player.getBasePosition().z) / Config::CELL_SIZE));
    if (lanesMoved > score) {
        score = lanesMoved;
        if (score > highScore) {
            highScore = score;
        }
    }
}
void Game::checkCollisions(float deltaTime)
{
    glm::vec3 playerPos = player.getPosition();
    glm::vec3 playerSize = player.getSize();
    glm::vec3 playerBasePos = player.getBasePosition();

    bool onLog = false;
    bool onLilypad = false;
    Lane *currentLane = nullptr;

    for (auto &lane : lanes)
    {
        if (std::abs(lane.getZPosition() - playerPos.z) < Config::CELL_SIZE / 2.0f)
        {
            currentLane = &lane;
            break;
        }
    }
    if (!currentLane)
        return;

    for (auto &lane : lanes)
    {
        if (lane.getType() != LANE_RAIL)
            continue;
        if (std::abs(lane.getZPosition() - playerPos.z) > Config::CELL_SIZE * 2.0f)
            continue;

        for (auto &obs : lane.getObstacles())
        {
            if (obs.getType() != OBSTACLE_TRAIN || !obs.getIsActive())
                continue;

            glm::vec3 obsPos = obs.getPosition();
            glm::vec3 obsSize = obs.getSize();
            bool hitX = std::abs(playerBasePos.x - obsPos.x) < (playerSize.x + obsSize.x) * 0.5f * Config::HITBOX_PADDING;
            bool hitZ = std::abs(playerPos.z - obsPos.z) < Config::CELL_SIZE * 0.9f;

            if (hitX && hitZ)
            {
                player.setDead(true);
                deathPosition = playerPos;
                hasWaterDeath = false;
                state = GAME_STATE_GAME_OVER;
                SaveManager::saveData(totalCoins, highScore);
                return;
            }
        }
    }

    for (auto &obs : currentLane->getObstacles())
    {
        if (!obs.getIsActive())
            continue;

        ObstacleType obsType = obs.getType();
        glm::vec3 obsPos = obs.getPosition();
        glm::vec3 obsSize = obs.getSize();

        bool isColliding = false;
        if (obsType == OBSTACLE_LOG || obsType == OBSTACLE_LILYPAD)
        {
            isColliding = Collision::checkAABB(
                playerBasePos, playerSize,
                obsPos, obsSize);
        }
        else
        {
            isColliding = Collision::checkAABB(
                playerPos, playerSize,
                obsPos, obsSize);
        }

        if (isColliding)
        {
            if (obsType == OBSTACLE_CAR || obsType == OBSTACLE_TRAIN)
            {
                player.setDead(true);
                deathPosition = playerPos;
                hasWaterDeath = false;
                state = GAME_STATE_GAME_OVER;
                SaveManager::saveData(totalCoins, highScore);
                return;
            }
            else if (obsType == OBSTACLE_LOG)
            {
                onLog = true;
                obs.setSinking(true);
                player.applyLogVelocity(obs.getSpeed(), deltaTime);

                if (std::abs(obsPos.x) > Config::LOG_STREAM_TRIGGER_X)
                {
                    obs.setFastStream(true);
                }

                if (std::abs(playerPos.x) > Config::BOUNDARY_X + 0.5f)
                {
                    const float waterSurface = -Config::CELL_SIZE * 0.1f;
                    player.triggerWaterDeath(waterSurface);
                    deathPosition = playerPos;
                    hasWaterDeath = true;
                    hasStreamDeath = true;
                    state = GAME_STATE_GAME_OVER;
                    SaveManager::saveData(totalCoins, highScore);
                    return;
                }
            }
            else if (obsType == OBSTACLE_LILYPAD)
            {
                onLilypad = true;
            }
        }
    }

    for (auto &coin : currentLane->coins)
    {
        if (coin.collected)
            continue;
        glm::vec3 coinPos = coin.getPosition();
        float coinSize = coin.getSize();
        if (std::abs(playerPos.x - coinPos.x) < playerSize.x &&
            std::abs(playerPos.z - coinPos.z) < playerSize.z)
        {
            coin.collected = true;
            coinScore += 10;
            totalCoins += 10; 
        }
    }

    LaneType laneType = currentLane->getType();
    if ((laneType == LANE_RIVER || laneType == LANE_LILYPAD) && !player.getIsJumping() && !onLog && !onLilypad)
    {
        const float waterSurface = -Config::CELL_SIZE * 0.1f;
        player.triggerWaterDeath(waterSurface);
        deathPosition = playerPos;
        hasWaterDeath = true;
        hasStreamDeath = false;
        state = GAME_STATE_GAME_OVER;
        SaveManager::saveData(totalCoins, highScore);
    }
}

void Game::renderWorldBoundaries()
{
    const glm::vec3 playerPos = player.getPosition();

    // Snap the Z calculation to the grid so mountains don't slide as the player smoothly jumps
    float snappedZ = std::round(playerPos.z / Config::CELL_SIZE) * Config::CELL_SIZE;
    
    // Z range to render: a little behind the player to well ahead.
    const float zStart = snappedZ + 9.0f * Config::CELL_SIZE;
    const float zEnd = snappedZ - 38.0f * Config::CELL_SIZE;
    const float step = Config::CELL_SIZE;

    struct SliceInfo
    {
        float z;
        LaneType ltype;
        float logFlowDir;
    };

    std::vector<SliceInfo> slices;
    slices.reserve(56);

    for (float z = zStart; z >= zEnd; z -= step)
    {
        LaneType ltype = LANE_GRASS;
        float lfd = 0.0f;

        for (const auto &lane : lanes)
        {
            if (std::abs(lane.getZPosition() - z) < step * 0.55f)
            {
                ltype = lane.getType();
                if (ltype == LANE_RIVER || ltype == LANE_LILYPAD)
                {
                    for (const auto &obs : lane.getObstacles())
                    {
                        if (obs.getIsActive())
                        {
                            lfd = (obs.getSpeed() >= 0.0f) ? 1.0f : -1.0f;
                            break;
                        }
                    }
                }
                break;
            }
        }
        slices.push_back({z, ltype, lfd});
    }

    const int nSlices = static_cast<int>(slices.size());
    for (int i = 0; i < nSlices; i++)
    {
        float z = slices[i].z;
        LaneType ltype = slices[i].ltype;
        float lfd = slices[i].logFlowDir;
        bool isTunnel = (ltype == LANE_ROAD || ltype == LANE_RAIL);

        bool isPortalFace = true;

        if (isTunnel)
        {
            LaneType prev = (i > 0) ? slices[i - 1].ltype : LANE_GRASS;
            LaneType next = (i < nSlices - 1) ? slices[i + 1].ltype : LANE_GRASS;

            bool isEntrance = (prev != ltype);
            isPortalFace = isEntrance;
        }

        renderer.drawMountainSection(z, ltype, lfd, isPortalFace);
    }

    for (const auto &lane : lanes)
    {
        LaneType lt = lane.getType();
        if (lt != LANE_RIVER && lt != LANE_LILYPAD)
            continue;
        float z = lane.getZPosition();
        if (z > playerPos.z + 10.0f || z < playerPos.z - 38.0f)
            continue;

        float foamY = -0.06f;
        renderer.drawFoam({-Config::BOUNDARY_X + 0.25f, foamY, z}, 1.2f, 0.9f);
        renderer.drawFoam({Config::BOUNDARY_X - 0.25f, foamY, z}, 1.2f, 0.9f);
    }

    renderer.drawBackWall();
}

void Game::updateDayNightCycle(float deltaTime)
{
    // Update game time for day/night cycle
    currentGameTime += deltaTime;
    
    // Normalize to 0-1 based on the time speed parameter
    float cycleTime = std::fmod(currentGameTime * Config::TIME_SPEED, 1.0f);
    
    // Calculate sun angle for shadow rendering
    // Sun arc expanded to full 180°: -π to +π (sun below horizon to above horizon to below horizon)
    // This allows proper day/night cycle where sun actually goes below horizon
    // cycleTime 0.0 → sunAngle = -π (sun fully below horizon, deep night)
    // cycleTime 0.25 → sunAngle = -π/4 (sun at horizon, night/day transition)
    // cycleTime 0.5 → sunAngle = 0 (sun at peak, noon)
    // cycleTime 0.75 → sunAngle = π/4 (sun at horizon, day/night transition)
    // cycleTime 1.0 → sunAngle = π (sun fully below horizon, deep night)
    sunAngle = (cycleTime - 0.5f) * 3.14159f;  // Full π range instead of π/2
    
    // Update renderer lighting
    renderer.updateLighting(currentGameTime);
    
    // Day/night mode based on sun angle relative to horizon
    // HORIZON_ANGLE = π/4 is where sun reaches horizon and shadows fully fade
    // Day: |sunAngle| < π/4 (sun above horizon, shadows visible)
    // Night: |sunAngle| >= π/4 (sun at/below horizon, no shadows)
    // This ensures day/night transitions happen EXACTLY when shadow fade completes
    const float HORIZON_ANGLE = 3.14159f / 4.0f;  // π/4 radians
    float sunAngleMagnitude = std::abs(sunAngle);
    bool isDayTime = (sunAngleMagnitude < HORIZON_ANGLE);
    renderer.setNightMode(!isDayTime);
}

void Game::renderShadows()
{
    // Only render shadows during gameplay
    if (state != GAME_STATE_PLAYING)
        return;
    
    // OPTIMIZATION: Calculate fade factor and sun angle magnitude ONCE per frame
    float sunAngleMagnitude = std::abs(sunAngle);
    float shadowFadeFactor = renderer.getShadowFadeFactor(sunAngle);
    
    // Skip shadow rendering if not enough light
    if (shadowFadeFactor <= 0.0f)
        return;
    
    // OPTIMIZED: Set up GL state once, then render all shadows
    glPushMatrix();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_TEXTURE_2D);
    
    // Render shadow for the player character
    glm::vec3 playerPos = player.getPosition();
    glm::vec3 playerSize = player.getSize();
    renderer.drawCharacterShadow(playerPos, playerSize, sunAngle, sunAngleMagnitude, shadowFadeFactor, LANE_GRASS);
    
    // Visibility culling: Get player Z position to only render nearby obstacles
    float playerZ = playerPos.z;
    const float SHADOW_RENDER_DISTANCE = 20.0f;
    
    // Render shadows for all active obstacles within visibility range
    for (auto &lane : lanes)
    {
        LaneType laneType = lane.getType();
        
        for (auto &obs : lane.getObstacles())
        {
            if (!obs.getIsActive())
                continue;
            
            glm::vec3 obsPos = obs.getPosition();
            
            // OPTIMIZATION: Skip obstacles that are too far away
            float distanceFromPlayer = std::abs(obsPos.z - playerZ);
            if (distanceFromPlayer > SHADOW_RENDER_DISTANCE)
                continue;
            
            // Render shadows for cars, trains, logs, and lilypads
            ObstacleType obsType = obs.getType();
            if (obsType == OBSTACLE_CAR || obsType == OBSTACLE_TRAIN || 
                obsType == OBSTACLE_LOG || obsType == OBSTACLE_LILYPAD)
            {
                glm::vec3 obsSize = obs.getSize();
                renderer.drawObstacleShadow(obsPos, obsSize, sunAngle, sunAngleMagnitude, shadowFadeFactor, laneType);
            }
        }
        
        // Render shadows for decorations (trees and rocks)
        if (laneType == LANE_GRASS)
        {
            for (const auto &dec : lane.decorations)
            {
                glm::vec3 decPos = dec.position;
                
                // OPTIMIZATION: Skip decorations that are too far away
                float distanceFromPlayer = std::abs(decPos.z - playerZ);
                if (distanceFromPlayer > SHADOW_RENDER_DISTANCE)
                    continue;
                
                // Render shadows for trees (type 0) and rocks (type 1)
                if (dec.type == 0)
                {
                    // Tree shadow
                    renderer.drawTreeShadow(decPos, dec.scale, sunAngle, sunAngleMagnitude, shadowFadeFactor, laneType);
                }
                else if (dec.type == 1)
                {
                    // Rock shadow
                    renderer.drawRockShadow(decPos, dec.scale, sunAngle, sunAngleMagnitude, shadowFadeFactor, laneType);
                }
            }
        }
    }
    
    // Restore GL state
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

void Game::render()
{
    renderer.prepareFrame();
    camera.apply();

    renderWorldBoundaries();

    for (auto &lane : lanes)
        lane.render(renderer, sunAngle);

    renderShadows();

    if (state == GAME_STATE_MAIN_MENU || state == GAME_STATE_START_SCREEN || state == GAME_STATE_CHARACTER_SELECT)
    {
        glm::vec3 pos = player.getPosition();
        int currentTime = glutGet(GLUT_ELAPSED_TIME);
        float timeSinceClick = (currentTime - lastClickTime) / 1000.0f;

        float wobble = 0.0f;
        if (eggClicks > 0 && timeSinceClick < 0.25f)
        {
            wobble = std::sin(timeSinceClick * 50.0f) * 0.3f;
            wobble *= (1.0f - timeSinceClick / 0.25f);
        }

        glPushMatrix();
        glTranslatef(pos.x, pos.y + 0.6f, pos.z);
        glRotatef(wobble * 45.0f, 0, 0, 1);
        glScalef(0.5f, 0.5f, 0.5f);
        renderer.drawEgg(eggClicks);
        glPopMatrix();
    }
    else if (state == GAME_STATE_PLAYING)
    {
        int currentTime = glutGet(GLUT_ELAPSED_TIME);
        float timeSinceStart = (currentTime - lastClickTime) / 1000.0f;
        float spawnDuration = 0.4f;

        if (timeSinceStart < spawnDuration)
        {
            float t = timeSinceStart / spawnDuration;
            float t1 = t - 1.0f;
            float scale = t1 * t1 * (2.5f * t1 + 1.5f) + 1.0f;
            if (scale < 0.0f) scale = 0.0f;

            glm::vec3 pos = player.getPosition();
            glPushMatrix();
            glTranslatef(pos.x, pos.y, pos.z);
            glScalef(scale, scale, scale);
            glTranslatef(-pos.x, -pos.y, -pos.z);
            player.render(renderer);
            glPopMatrix();
        }
        else
        {
            player.render(renderer);
        }
    }

    else if (state == GAME_STATE_GAME_OVER)
    {
        // Render player death animation
        player.render(renderer);
    }

    camera.renderOverlay(windowWidth, windowHeight);
    renderUIOverlay();
}

void Game::onKeyPress(unsigned char key)
{
    // Delegate pre-game keyboard input to PreGameManager
    preGameManager.onKeyPress(key, state, selectedCharacterIndex, player);

    if (state != GAME_STATE_PLAYING)
        return;

    float dx = 0.0f, dz = 0.0f;
    if (key == 'w' || key == 'W')
        dz = -1.0f;
    if (key == 's' || key == 'S')
        dz = 1.0f;
    if (key == 'a' || key == 'A')
        dx = -1.0f;
    if (key == 'd' || key == 'D')
        dx = 1.0f;

    if (dx == 0.0f && dz == 0.0f)
    {
        if (key == 'v' || key == 'V')
            camera.cyclePreset();
        if (key == 'c' || key == 'C')
            camera.toggleLock();
        return;
    }

    glm::vec3 currentPos = player.getPosition();
    glm::vec3 nextPos = currentPos + glm::vec3(dx * Config::CELL_SIZE,
                                               0.0f,
                                               dz * Config::CELL_SIZE);

    if (std::abs(nextPos.x) >= Config::BOUNDARY_X)
    {
        if (key == 'v' || key == 'V')
            camera.cyclePreset();
        if (key == 'c' || key == 'C')
            camera.toggleLock();
        return;
    }

    if (nextPos.z > Config::BOUNDARY_BACK_Z)
    {
        if (key == 'v' || key == 'V')
            camera.cyclePreset();
        if (key == 'c' || key == 'C')
            camera.toggleLock();
        return;
    }

    Lane *targetLane = nullptr;
    for (auto &lane : lanes)
    {
        if (std::abs(lane.getZPosition() - nextPos.z) < Config::CELL_SIZE / 2.0f)
        {
            targetLane = &lane;
            break;
        }
    }

    bool blocked = false;

    if (targetLane && targetLane->getType() == LANE_GRASS)
    {
        for (auto &d : targetLane->decorations)
        {
            float sz = (d.type == 0) ? 0.6f : 0.5f;
            if (std::abs(nextPos.x - d.position.x) < sz &&
                std::abs(nextPos.z - d.position.z) < sz)
            {
                blocked = true;
                break;
            }
        }
    }

    // if (!blocked && targetLane && targetLane->getType() == LANE_RAIL)
    // {
    //     for (const auto &sp : targetLane->signalPosts)
    //     {
    //         if (std::abs(nextPos.x - sp.position.x) < 0.45f)
    //         {
    //             blocked = true;
    //             break;
    //         }
    //     }
    // }

    if (!blocked)
        player.move(dx, dz);

    if (key == 'v' || key == 'V')
        camera.cyclePreset();
    if (key == 'c' || key == 'C')
        camera.toggleLock();
}

void Game::onSpecialKey(int key)
{
    // Delegate pre-game special key input to PreGameManager
    preGameManager.onSpecialKey(key, state, selectedCharacterIndex);
}

void Game::onMouseDrag(float deltaX, float deltaY)
{
    if (state != GAME_STATE_PLAYING)
        return;
    camera.processMouseDrag(deltaX, deltaY);
}

void Game::onResize(int w, int h)
{
    windowWidth = w;
    windowHeight = h;
}
void Game::renderUIOverlay()
{
    // Delegate pre-game and game-over rendering to PreGameManager
    preGameManager.render(state, player, windowWidth, windowHeight,
                         eggClicks, lastClickTime, selectedCharacterIndex,
                         highScore, totalCoins, score, coinScore);
    
    // In-game HUD rendering (score and coins)
    if (state == GAME_STATE_PLAYING)
    {
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
        
        auto fillRect = [](float x1, float y1, float x2, float y2,
                           float r, float g, float b, float a = 1.0f) {
            glColor4f(r, g, b, a);
            glBegin(GL_QUADS);
            glVertex2f(x1, y1); glVertex2f(x2, y1);
            glVertex2f(x2, y2); glVertex2f(x1, y2);
            glEnd();
        };
        
        auto drawText = [](float x, float y, const std::string& txt, void* font,
                           float r, float g, float b, float a = 1.0f, bool shadow = false) {
            if (shadow) {
                glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
                glRasterPos2f(x + 2.0f, y - 2.0f);
                for (char c : txt) glutBitmapCharacter(font, c);
            }
            glColor4f(r, g, b, a);
            glRasterPos2f(x, y);
            for (char c : txt) glutBitmapCharacter(font, c);
        };
        
        auto drawCircle = [](float cx_, float cy_, float r, float red, float grn, float blu, float a, int segs = 16) {
            glColor4f(red, grn, blu, a);
            glBegin(GL_POLYGON);
            for (int i = 0; i < segs; i++) {
                float ang = i * 2.0f * 3.14159f / segs;
                glVertex2f(cx_ + r * std::cos(ang), cy_ + r * std::sin(ang));
            }
            glEnd();
        };
        
        // Score pill (top-right)
        std::stringstream ss; ss << score;
        float sw = 0; for (char c : ss.str()) sw += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, c);
        fillRect(windowWidth - sw - 54, windowHeight - 62, windowWidth - 12, windowHeight - 18, 0, 0, 0, 0.45f);
        fillRect(windowWidth - sw - 52, windowHeight - 60, windowWidth - 14, windowHeight - 20, 0.05f, 0.08f, 0.18f, 0.80f);
        drawText(windowWidth - sw - 38, windowHeight - 47, ss.str(), GLUT_BITMAP_TIMES_ROMAN_24, 1.0f, 1.0f, 1.0f, 1.0f, true);
        
        // Coin pill (top-left)
        float coinX = 36.0f, coinY = windowHeight - 40.0f;
        std::stringstream cc; cc << coinScore;
        float cw2 = 0; for (char c : cc.str()) cw2 += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        fillRect(10, coinY - 16, coinX + cw2 + 38, coinY + 16, 0, 0, 0, 0.45f);
        fillRect(12, coinY - 14, coinX + cw2 + 36, coinY + 14, 0.10f, 0.08f, 0.01f, 0.80f);
        drawCircle(coinX, coinY, 11.0f, 1.0f, 0.82f, 0.0f, 1.0f, 14);
        drawCircle(coinX, coinY,  7.0f, 0.82f, 0.65f, 0.0f, 1.0f, 14);
        drawText(coinX + 16, coinY - 7, cc.str(), GLUT_BITMAP_HELVETICA_18, 1.0f, 0.88f, 0.0f, 1.0f, true);
        
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
}
void Game::onMouseClick(int button, int clickState, int x, int y)
{
    // Handle game over → main menu reset
    if (state == GAME_STATE_GAME_OVER) {
        float btnW = 230.0f, btnH = 62.0f;
        float cx = windowWidth / 2.0f;
        float cy = windowHeight / 2.0f;
        float retryY = cy - 98.0f;
        int invertedY = windowHeight - y;
        if (x > cx - btnW/2 && x < cx + btnW/2 &&
            invertedY > retryY - btnH/2 && invertedY < retryY + btnH/2) {
            resetGame();
            return;  // Don't pass this click to PreGameManager
        }
    }
    
    preGameManager.onMouseClick(button, clickState, x, y, state, selectedCharacterIndex,
                                player, windowWidth, windowHeight, eggClicks, lastClickTime);
}