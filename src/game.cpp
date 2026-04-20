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
      currentGenerationZ(5.0f * Config::CELL_SIZE),
      cameraTrackZ(0.0f), score(0), highScore(0), startZ(0.0f), coinScore(0), totalCoins(0), // <-- Add totalCoins(0)
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
    currentGameTime = 30.0f;  // Start at midday (daytime) for better gameplay visibility 

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

    state = GAME_STATE_MAIN_MENU; // Ensure we always return to the menu after a reset
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

    if (state == GAME_STATE_START_SCREEN) {
        if (key == '1') player.setModel(MODEL_CHICKEN);
        if (key == '2') player.setModel(MODEL_FROG);
        if (key == '3') player.setModel(MODEL_DINO); // <--- ADDED
        if (key == '4') player.setModel(MODEL_CAT);  // <--- ADDED
        if (key == '5') player.setModel(MODEL_DOG);  // <--- ADDED
    }

    // Enter key selects character on the character select screen
    if (state == GAME_STATE_CHARACTER_SELECT && (key == '\r' || key == '\n')) {
        decltype(MODEL_CHICKEN) models[] = { MODEL_CHICKEN, MODEL_FROG, MODEL_DINO, MODEL_CAT, MODEL_DOG };
        player.setModel(models[selectedCharacterIndex]);
        state = GAME_STATE_MAIN_MENU;
    }

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
    if (state != GAME_STATE_CHARACTER_SELECT)
        return;
    const int numChars = 5;
    if (key == GLUT_KEY_LEFT)
        selectedCharacterIndex = (selectedCharacterIndex - 1 + numChars) % numChars;
    else if (key == GLUT_KEY_RIGHT)
        selectedCharacterIndex = (selectedCharacterIndex + 1) % numChars;
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
 
    float cx = windowWidth / 2.0f;
    float cy = windowHeight / 2.0f;
    int timeMs = glutGet(GLUT_ELAPSED_TIME);
 
    // ── Helpers ─────────────────────────────────────────────────────────────
 
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
 
    // Centre text on cx
    auto drawCentered = [&](float tcx, float y, const std::string& txt, void* font,
                            float r, float g, float b, bool shadow = false) {
        float w = 0; for (char c : txt) w += glutBitmapWidth(font, c);
        drawText(tcx - w / 2.0f, y, txt, font, r, g, b, 1.0f, shadow);
    };
 
    // Stylish 3-layer button with top highlight
    auto drawButton = [&](float bcx, float bcy, float bw, float bh,
                          float br, float bg, float bb,
                          float sr, float sg, float sb,
                          const std::string& label, void* font) {
        // Drop shadow
        fillRect(bcx - bw/2 + 5, bcy - bh/2 - 5, bcx + bw/2 + 5, bcy + bh/2 - 5, 0, 0, 0, 0.45f);
        // Dark border
        fillRect(bcx - bw/2 - 2, bcy - bh/2 - 2, bcx + bw/2 + 2, bcy + bh/2 + 2, sr*0.6f, sg*0.6f, sb*0.6f);
        // Bottom edge (gives 3-D depth)
        fillRect(bcx - bw/2, bcy - bh/2 - 4, bcx + bw/2, bcy + bh/2, sr, sg, sb);
        // Main face
        fillRect(bcx - bw/2, bcy - bh/2, bcx + bw/2, bcy + bh/2, br, bg, bb);
        // Top sheen
        fillRect(bcx - bw/2 + 3, bcy + bh/2 - 9, bcx + bw/2 - 3, bcy + bh/2 - 3,
                 std::min(1.0f, br * 1.55f), std::min(1.0f, bg * 1.55f), std::min(1.0f, bb * 1.55f), 0.55f);
        // Label
        float lw = 0; for (char c : label) lw += glutBitmapWidth(font, c);
        drawText(bcx - lw/2.0f, bcy - 8.0f, label, font, 1.0f, 1.0f, 1.0f, 1.0f, true);
    };
 
    // Circle approximated with GL_POLYGON (n segments)
    auto drawCircle = [](float cx_, float cy_, float r, float red, float grn, float blu, float a, int segs = 16) {
        glColor4f(red, grn, blu, a);
        glBegin(GL_POLYGON);
        for (int i = 0; i < segs; i++) {
            float ang = i * 2.0f * 3.14159f / segs;
            glVertex2f(cx_ + r * std::cos(ang), cy_ + r * std::sin(ang));
        }
        glEnd();
    };
 
    // ── Character avatar (2-D voxel art) ────────────────────────────────────
    auto drawCharIcon = [&](float icx, float icy, float sz, int ci, bool selected) {
        float s = sz / 100.0f;
        float cw = sz * 1.25f, ch = sz * 1.65f;
 
        // Card drop shadow
        fillRect(icx - cw/2 + 6, icy - ch/2 - 6, icx + cw/2 + 6, icy + ch/2 - 6, 0, 0, 0, 0.45f);
 
        // Gold animated border for selected card
        if (selected) {
            float pulse = 0.65f + 0.35f * std::abs(std::sin(timeMs * 0.0035f));
            fillRect(icx - cw/2 - 5, icy - ch/2 - 5, icx + cw/2 + 5, icy + ch/2 + 5,
                     1.0f * pulse, 0.82f * pulse, 0.0f);
        }
 
        // Card fill – brighter for selected
        if (selected)
            fillRect(icx - cw/2, icy - ch/2, icx + cw/2, icy + ch/2, 0.12f, 0.30f, 0.58f, 0.92f);
        else
            fillRect(icx - cw/2, icy - ch/2, icx + cw/2, icy + ch/2, 0.06f, 0.12f, 0.25f, 0.80f);
 
        // Inner top sheen
        fillRect(icx - cw/2 + 3, icy + ch/2 - 10, icx + cw/2 - 3, icy + ch/2 - 3,
                 1.0f, 1.0f, 1.0f, selected ? 0.12f : 0.06f);
 
        switch (ci) {
            // ── CHICKEN ─────────────────────────────────────────────────────
            case 0: {
                // Feet
                glColor4f(1.0f, 0.55f, 0.0f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 21*s, icy - 44*s); glVertex2f(icx - 6*s,  icy - 44*s);
                glVertex2f(icx - 6*s,  icy - 28*s); glVertex2f(icx - 21*s, icy - 28*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 6*s,  icy - 44*s); glVertex2f(icx + 21*s, icy - 44*s);
                glVertex2f(icx + 21*s, icy - 28*s); glVertex2f(icx + 6*s,  icy - 28*s);
                glEnd();
                // Body
                glColor4f(1.0f, 0.95f, 0.88f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 23*s, icy - 30*s); glVertex2f(icx + 23*s, icy - 30*s);
                glVertex2f(icx + 21*s, icy + 12*s); glVertex2f(icx - 21*s, icy + 12*s);
                glEnd();
                // Head
                glBegin(GL_QUADS);
                glVertex2f(icx - 16*s, icy + 11*s); glVertex2f(icx + 16*s, icy + 11*s);
                glVertex2f(icx + 15*s, icy + 38*s); glVertex2f(icx - 15*s, icy + 38*s);
                glEnd();
                // Comb
                glColor4f(0.92f, 0.12f, 0.1f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx - 9*s, icy + 37*s); glVertex2f(icx - 3*s, icy + 52*s); glVertex2f(icx + 3*s, icy + 37*s);
                glVertex2f(icx + 2*s, icy + 37*s); glVertex2f(icx + 7*s, icy + 50*s); glVertex2f(icx + 12*s, icy + 37*s);
                glEnd();
                // Wattle
                glColor4f(0.9f, 0.15f, 0.1f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx - 8*s, icy + 14*s); glVertex2f(icx + 2*s, icy + 14*s); glVertex2f(icx - 3*s, icy + 7*s);
                glEnd();
                // Beak
                glColor4f(1.0f, 0.62f, 0.0f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx - 4*s, icy + 20*s); glVertex2f(icx + 13*s, icy + 22*s); glVertex2f(icx + 4*s, icy + 14*s);
                glEnd();
                // Eye dark
                glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx + 3*s, icy + 27*s); glVertex2f(icx + 11*s, icy + 27*s);
                glVertex2f(icx + 11*s, icy + 33*s); glVertex2f(icx + 3*s, icy + 33*s);
                glEnd();
                // Eye shine
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx + 4*s, icy + 30*s); glVertex2f(icx + 7*s, icy + 30*s);
                glVertex2f(icx + 7*s, icy + 33*s); glVertex2f(icx + 4*s, icy + 33*s);
                glEnd();
                break;
            }
            // ── FROG ────────────────────────────────────────────────────────
            case 1: {
                // Wide back legs
                glColor4f(0.14f, 0.62f, 0.14f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 40*s, icy - 35*s); glVertex2f(icx - 21*s, icy - 35*s);
                glVertex2f(icx - 21*s, icy - 5*s);  glVertex2f(icx - 40*s, icy - 5*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 21*s, icy - 35*s); glVertex2f(icx + 40*s, icy - 35*s);
                glVertex2f(icx + 40*s, icy - 5*s);  glVertex2f(icx + 21*s, icy - 5*s);
                glEnd();
                // Body
                glColor4f(0.18f, 0.78f, 0.18f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 26*s, icy - 28*s); glVertex2f(icx + 26*s, icy - 28*s);
                glVertex2f(icx + 27*s, icy + 16*s); glVertex2f(icx - 27*s, icy + 16*s);
                glEnd();
                // Belly
                glColor4f(0.62f, 0.96f, 0.52f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 17*s, icy - 23*s); glVertex2f(icx + 17*s, icy - 23*s);
                glVertex2f(icx + 16*s, icy + 12*s); glVertex2f(icx - 16*s, icy + 12*s);
                glEnd();
                // Wide head
                glColor4f(0.18f, 0.78f, 0.18f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 29*s, icy + 14*s); glVertex2f(icx + 29*s, icy + 14*s);
                glVertex2f(icx + 25*s, icy + 40*s); glVertex2f(icx - 25*s, icy + 40*s);
                glEnd();
                // Smile
                glColor4f(0.1f, 0.5f, 0.1f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 19*s, icy + 18*s); glVertex2f(icx + 19*s, icy + 18*s);
                glVertex2f(icx + 19*s, icy + 22*s); glVertex2f(icx - 19*s, icy + 22*s);
                glEnd();
                // Bulging eyes (white + pupil)
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 27*s, icy + 37*s); glVertex2f(icx - 11*s, icy + 37*s);
                glVertex2f(icx - 11*s, icy + 52*s); glVertex2f(icx - 27*s, icy + 52*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 11*s, icy + 37*s); glVertex2f(icx + 27*s, icy + 37*s);
                glVertex2f(icx + 27*s, icy + 52*s); glVertex2f(icx + 11*s, icy + 52*s);
                glEnd();
                glColor4f(0.0f, 0.18f, 0.0f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 24*s, icy + 39*s); glVertex2f(icx - 13*s, icy + 39*s);
                glVertex2f(icx - 13*s, icy + 50*s); glVertex2f(icx - 24*s, icy + 50*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 13*s, icy + 39*s); glVertex2f(icx + 24*s, icy + 39*s);
                glVertex2f(icx + 24*s, icy + 50*s); glVertex2f(icx + 13*s, icy + 50*s);
                glEnd();
                // Eye shine
                glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 24*s, icy + 48*s); glVertex2f(icx - 19*s, icy + 48*s);
                glVertex2f(icx - 19*s, icy + 52*s); glVertex2f(icx - 24*s, icy + 52*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 19*s, icy + 48*s); glVertex2f(icx + 24*s, icy + 48*s);
                glVertex2f(icx + 24*s, icy + 52*s); glVertex2f(icx + 19*s, icy + 52*s);
                glEnd();
                break;
            }
            // ── DINO (brown T-Rex, matches in-game model) ─────────────────────
            case 2: {
                // Tail (stubby, low-right)
                glColor4f(0.52f, 0.32f, 0.12f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx + 20*s, icy - 18*s);
                glVertex2f(icx + 46*s, icy - 30*s);
                glVertex2f(icx + 20*s, icy - 6*s);
                glEnd();
                // Left leg
                glColor4f(0.48f, 0.28f, 0.10f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 18*s, icy - 46*s); glVertex2f(icx - 4*s,  icy - 46*s);
                glVertex2f(icx - 4*s,  icy - 22*s); glVertex2f(icx - 18*s, icy - 22*s);
                glEnd();
                // Right leg
                glBegin(GL_QUADS);
                glVertex2f(icx + 4*s,  icy - 46*s); glVertex2f(icx + 18*s, icy - 46*s);
                glVertex2f(icx + 18*s, icy - 22*s); glVertex2f(icx + 4*s,  icy - 22*s);
                glEnd();
                // Feet (white claws)
                glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 20*s, icy - 50*s); glVertex2f(icx - 2*s,  icy - 50*s);
                glVertex2f(icx - 2*s,  icy - 44*s); glVertex2f(icx - 20*s, icy - 44*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 2*s,  icy - 50*s); glVertex2f(icx + 20*s, icy - 50*s);
                glVertex2f(icx + 20*s, icy - 44*s); glVertex2f(icx + 2*s,  icy - 44*s);
                glEnd();
                // Body
                glColor4f(0.62f, 0.38f, 0.15f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 22*s, icy - 24*s); glVertex2f(icx + 22*s, icy - 24*s);
                glVertex2f(icx + 22*s, icy + 18*s); glVertex2f(icx - 22*s, icy + 18*s);
                glEnd();
                // Belly (cream/tan patch)
                glColor4f(0.88f, 0.72f, 0.50f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 13*s, icy - 22*s); glVertex2f(icx + 13*s, icy - 22*s);
                glVertex2f(icx + 10*s, icy + 16*s); glVertex2f(icx - 10*s, icy + 16*s);
                glEnd();
                // Small arms
                glColor4f(0.55f, 0.33f, 0.12f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 22*s, icy + 2*s); glVertex2f(icx - 32*s, icy - 6*s);
                glVertex2f(icx - 30*s, icy - 12*s); glVertex2f(icx - 20*s, icy - 4*s);
                glEnd();
                // Head
                glColor4f(0.62f, 0.38f, 0.15f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 20*s, icy + 17*s); glVertex2f(icx + 20*s, icy + 17*s);
                glVertex2f(icx + 20*s, icy + 50*s); glVertex2f(icx - 20*s, icy + 50*s);
                glEnd();
                // Snout
                glColor4f(0.70f, 0.44f, 0.18f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx + 12*s, icy + 17*s); glVertex2f(icx + 28*s, icy + 17*s);
                glVertex2f(icx + 28*s, icy + 32*s); glVertex2f(icx + 12*s, icy + 32*s);
                glEnd();
                // Nostril
                glColor4f(0.40f, 0.22f, 0.08f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx + 21*s, icy + 28*s); glVertex2f(icx + 26*s, icy + 28*s);
                glVertex2f(icx + 26*s, icy + 31*s); glVertex2f(icx + 21*s, icy + 31*s);
                glEnd();
                // Eye (white + pupil)
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx + 4*s,  icy + 36*s); glVertex2f(icx + 16*s, icy + 36*s);
                glVertex2f(icx + 16*s, icy + 46*s); glVertex2f(icx + 4*s,  icy + 46*s);
                glEnd();
                glColor4f(0.08f, 0.08f, 0.08f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx + 8*s,  icy + 38*s); glVertex2f(icx + 14*s, icy + 38*s);
                glVertex2f(icx + 14*s, icy + 44*s); glVertex2f(icx + 8*s,  icy + 44*s);
                glEnd();
                // Teeth
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx + 14*s, icy + 17*s); glVertex2f(icx + 18*s, icy + 17*s); glVertex2f(icx + 16*s, icy + 11*s);
                glVertex2f(icx + 20*s, icy + 17*s); glVertex2f(icx + 24*s, icy + 17*s); glVertex2f(icx + 22*s, icy + 11*s);
                glEnd();
                break;
            }
            // ── CAT ──────────────────────────────────────────────────────────
            case 3: {
                // Tail
                glColor4f(0.88f, 0.48f, 0.14f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx + 21*s, icy + 12*s);
                glVertex2f(icx + 40*s, icy + 34*s);
                glVertex2f(icx + 21*s, icy + 0*s);
                glEnd();
                // Legs
                glColor4f(0.84f, 0.44f, 0.10f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 21*s, icy - 42*s); glVertex2f(icx - 7*s, icy - 42*s);
                glVertex2f(icx - 7*s,  icy - 20*s); glVertex2f(icx - 21*s, icy - 20*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 7*s,  icy - 42*s); glVertex2f(icx + 21*s, icy - 42*s);
                glVertex2f(icx + 21*s, icy - 20*s); glVertex2f(icx + 7*s,  icy - 20*s);
                glEnd();
                // Body
                glColor4f(0.92f, 0.52f, 0.16f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 22*s, icy - 22*s); glVertex2f(icx + 22*s, icy - 22*s);
                glVertex2f(icx + 22*s, icy + 18*s); glVertex2f(icx - 22*s, icy + 18*s);
                glEnd();
                // Head
                glBegin(GL_QUADS);
                glVertex2f(icx - 20*s, icy + 17*s); glVertex2f(icx + 20*s, icy + 17*s);
                glVertex2f(icx + 20*s, icy + 44*s); glVertex2f(icx - 20*s, icy + 44*s);
                glEnd();
                // Ears outer
                glColor4f(0.84f, 0.43f, 0.11f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx - 20*s, icy + 42*s); glVertex2f(icx - 9*s, icy + 42*s); glVertex2f(icx - 16*s, icy + 60*s);
                glVertex2f(icx + 9*s,  icy + 42*s); glVertex2f(icx + 20*s, icy + 42*s); glVertex2f(icx + 16*s, icy + 60*s);
                glEnd();
                // Ears inner (pink)
                glColor4f(1.0f, 0.70f, 0.80f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx - 18*s, icy + 43*s); glVertex2f(icx - 11*s, icy + 43*s); glVertex2f(icx - 16*s, icy + 56*s);
                glVertex2f(icx + 11*s, icy + 43*s); glVertex2f(icx + 18*s, icy + 43*s); glVertex2f(icx + 16*s, icy + 56*s);
                glEnd();
                // Eyes (slit pupils)
                glColor4f(0.08f, 0.70f, 0.08f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 15*s, icy + 30*s); glVertex2f(icx - 6*s, icy + 30*s);
                glVertex2f(icx - 6*s,  icy + 38*s); glVertex2f(icx - 15*s, icy + 38*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 6*s,  icy + 30*s); glVertex2f(icx + 15*s, icy + 30*s);
                glVertex2f(icx + 15*s, icy + 38*s); glVertex2f(icx + 6*s,  icy + 38*s);
                glEnd();
                glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 12*s, icy + 31*s); glVertex2f(icx - 8*s, icy + 31*s);
                glVertex2f(icx - 8*s,  icy + 37*s); glVertex2f(icx - 12*s, icy + 37*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 8*s,  icy + 31*s); glVertex2f(icx + 12*s, icy + 31*s);
                glVertex2f(icx + 12*s, icy + 37*s); glVertex2f(icx + 8*s,  icy + 37*s);
                glEnd();
                // Nose
                glColor4f(1.0f, 0.48f, 0.68f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx - 3*s, icy + 24*s); glVertex2f(icx + 3*s, icy + 24*s); glVertex2f(icx, icy + 20*s);
                glEnd();
                // Whiskers
                glColor4f(1.0f, 1.0f, 1.0f, 0.88f);
                glLineWidth(1.2f);
                glBegin(GL_LINES);
                glVertex2f(icx - 4*s, icy + 24*s); glVertex2f(icx - 22*s, icy + 25*s);
                glVertex2f(icx - 4*s, icy + 21*s); glVertex2f(icx - 22*s, icy + 18*s);
                glVertex2f(icx + 4*s, icy + 24*s); glVertex2f(icx + 22*s, icy + 25*s);
                glVertex2f(icx + 4*s, icy + 21*s); glVertex2f(icx + 22*s, icy + 18*s);
                glEnd();
                glLineWidth(1.0f);
                break;
            }
            // ── DOG ──────────────────────────────────────────────────────────
            case 4: {
                // Wagging tail
                float waggle = std::sin(timeMs * 0.007f) * 10.0f * s;
                glColor4f(0.58f, 0.36f, 0.18f, 1.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(icx + 22*s, icy + 20*s);
                glVertex2f(icx + 42*s, icy + 34*s + waggle);
                glVertex2f(icx + 22*s, icy + 4*s);
                glEnd();
                // Legs
                glColor4f(0.56f, 0.34f, 0.16f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 24*s, icy - 43*s); glVertex2f(icx - 8*s,  icy - 43*s);
                glVertex2f(icx - 8*s,  icy - 22*s); glVertex2f(icx - 24*s, icy - 22*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 8*s,  icy - 43*s); glVertex2f(icx + 24*s, icy - 43*s);
                glVertex2f(icx + 24*s, icy - 22*s); glVertex2f(icx + 8*s,  icy - 22*s);
                glEnd();
                // Body
                glColor4f(0.66f, 0.42f, 0.20f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 25*s, icy - 24*s); glVertex2f(icx + 25*s, icy - 24*s);
                glVertex2f(icx + 25*s, icy + 20*s); glVertex2f(icx - 25*s, icy + 20*s);
                glEnd();
                // Floppy ears (hang beside head)
                glColor4f(0.48f, 0.28f, 0.12f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 36*s, icy + 16*s); glVertex2f(icx - 23*s, icy + 16*s);
                glVertex2f(icx - 23*s, icy + 46*s); glVertex2f(icx - 36*s, icy + 46*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 23*s, icy + 16*s); glVertex2f(icx + 36*s, icy + 16*s);
                glVertex2f(icx + 36*s, icy + 46*s); glVertex2f(icx + 23*s, icy + 46*s);
                glEnd();
                // Head
                glColor4f(0.70f, 0.45f, 0.22f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 23*s, icy + 18*s); glVertex2f(icx + 23*s, icy + 18*s);
                glVertex2f(icx + 23*s, icy + 48*s); glVertex2f(icx - 23*s, icy + 48*s);
                glEnd();
                // Eyes
                glColor4f(0.22f, 0.10f, 0.02f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 16*s, icy + 33*s); glVertex2f(icx - 6*s,  icy + 33*s);
                glVertex2f(icx - 6*s,  icy + 42*s); glVertex2f(icx - 16*s, icy + 42*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 6*s,  icy + 33*s); glVertex2f(icx + 16*s, icy + 33*s);
                glVertex2f(icx + 16*s, icy + 42*s); glVertex2f(icx + 6*s,  icy + 42*s);
                glEnd();
                // Eye shine
                glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 15*s, icy + 39*s); glVertex2f(icx - 11*s, icy + 39*s);
                glVertex2f(icx - 11*s, icy + 42*s); glVertex2f(icx - 15*s, icy + 42*s);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(icx + 7*s,  icy + 39*s); glVertex2f(icx + 11*s, icy + 39*s);
                glVertex2f(icx + 11*s, icy + 42*s); glVertex2f(icx + 7*s,  icy + 42*s);
                glEnd();
                // Nose
                glColor4f(0.08f, 0.04f, 0.04f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 7*s, icy + 23*s); glVertex2f(icx + 7*s, icy + 23*s);
                glVertex2f(icx + 7*s, icy + 28*s); glVertex2f(icx - 7*s, icy + 28*s);
                glEnd();
                // Tongue
                glColor4f(1.0f, 0.40f, 0.50f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(icx - 6*s, icy + 15*s); glVertex2f(icx + 6*s, icy + 15*s);
                glVertex2f(icx + 6*s, icy + 22*s); glVertex2f(icx - 6*s, icy + 22*s);
                glEnd();
                break;
            }
        }
    };
 
    // ── MAIN MENU ────────────────────────────────────────────────────────────
    if (state == GAME_STATE_MAIN_MENU)
    {
        // ── Coin bank (top-left) ─────────────────────────────────────────────
        float coinX = 38.0f, coinY = windowHeight - 38.0f;
        // Pill background
        float pillW = 110.0f;
        fillRect(coinX - 18, coinY - 16, coinX + pillW, coinY + 16, 0.0f, 0.0f, 0.0f, 0.45f);
        fillRect(coinX - 16, coinY - 14, coinX + pillW - 2, coinY + 14, 0.12f, 0.10f, 0.02f, 0.80f);
        // Coin circle
        drawCircle(coinX, coinY, 12.0f, 1.0f, 0.82f, 0.0f, 1.0f, 14);
        drawCircle(coinX, coinY,  8.0f, 0.82f, 0.65f, 0.0f, 1.0f, 14);
        std::stringstream tc; tc << totalCoins;
        drawText(coinX + 18, coinY - 7, tc.str(), GLUT_BITMAP_HELVETICA_18, 1.0f, 0.9f, 0.1f, 1.0f, true);
 
        // ── Best score (top-right) ────────────────────────────────────────────
        std::stringstream hs; hs << "BEST  " << highScore;
        float hsW = 0; for (char c : hs.str()) hsW += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        float hsX = windowWidth - hsW - 38.0f;
        fillRect(hsX - 14, coinY - 16, windowWidth - 12, coinY + 16, 0.0f, 0.0f, 0.0f, 0.45f);
        fillRect(hsX - 12, coinY - 14, windowWidth - 14, coinY + 14, 0.05f, 0.05f, 0.12f, 0.80f);
        // Trophy icon
        glColor4f(1.0f, 0.80f, 0.0f, 1.0f);
        fillRect(hsX - 10, coinY - 9, hsX + 4,  coinY + 9, 1.0f, 0.80f, 0.0f, 1.0f);  // cup body
        fillRect(hsX - 12, coinY + 5, hsX + 6,  coinY + 9, 1.0f, 0.80f, 0.0f, 1.0f);  // rim
        fillRect(hsX - 5,  coinY - 12, hsX + 0, coinY - 9, 1.0f, 0.80f, 0.0f, 1.0f);  // stem
        fillRect(hsX - 8,  coinY - 14, hsX + 4, coinY - 11, 1.0f, 0.80f, 0.0f, 1.0f); // base
        drawText(hsX + 8, coinY - 7, hs.str(), GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, 1.0f, true);
 
        // ── Title banner ─────────────────────────────────────────────────────
        float titleY = cy + 118.0f;
        float bw = 200.0f, bh = 30.0f;
        // Shadow
        fillRect(cx - bw - 6, titleY - bh - 6, cx + bw + 6, titleY + bh - 2, 0, 0, 0, 0.45f);
        // Outer border
        fillRect(cx - bw - 3, titleY - bh - 3, cx + bw + 3, titleY + bh + 3, 0.05f, 0.10f, 0.22f);
        // Gold top/bottom pinstripes
        fillRect(cx - bw - 3, titleY + bh - 2, cx + bw + 3, titleY + bh + 3, 1.0f, 0.82f, 0.0f);
        fillRect(cx - bw - 3, titleY - bh - 3, cx + bw + 3, titleY - bh + 2, 1.0f, 0.82f, 0.0f);
        // Gradient body (two-tone illusion)
        glColor4f(0.12f, 0.28f, 0.58f, 0.96f);
        glBegin(GL_QUADS);
        glVertex2f(cx - bw, titleY - bh); glVertex2f(cx + bw, titleY - bh);
        glVertex2f(cx + bw, titleY);       glVertex2f(cx - bw, titleY);
        glEnd();
        glColor4f(0.20f, 0.48f, 0.88f, 0.96f);
        glBegin(GL_QUADS);
        glVertex2f(cx - bw, titleY); glVertex2f(cx + bw, titleY);
        glVertex2f(cx + bw, titleY + bh); glVertex2f(cx - bw, titleY + bh);
        glEnd();
        // Title text (bigger font - use 2 shadows for depth)
        {
            std::string title = "CRAZY  HOPPER";
            float tw = 0; for (char c : title) tw += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, c);
            // Deep shadow
            glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
            glRasterPos2f(cx - tw/2 + 3, titleY - 8); for (char c : title) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
            // Mid shadow
            glColor4f(0.0f, 0.2f, 0.5f, 0.8f);
            glRasterPos2f(cx - tw/2 + 1, titleY - 6); for (char c : title) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
            // Main text
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glRasterPos2f(cx - tw/2, titleY - 7); for (char c : title) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        }
 
        // ── PLAY button (large, pulsing green) ───────────────────────────────
        float playPulse = 3.0f * std::abs(std::sin(timeMs * 0.0022f));
        drawButton(cx, cy - 18.0f, 230.0f + playPulse, 64.0f,
                   0.08f, 0.74f, 0.08f,
                   0.04f, 0.38f, 0.04f,
                   "PLAY!", GLUT_BITMAP_TIMES_ROMAN_24);
 
        // Blinking "tap anywhere" hint
        if ((timeMs / 600) % 2 == 0) {
            drawCentered(cx, cy - 100.0f, "- TAP ANYWHERE TO START -",
                         GLUT_BITMAP_HELVETICA_12, 0.88f, 0.88f, 0.88f, false);
        }
 
        // ── Characters button (bottom-left) ──────────────────────────────────
        drawButton(72.0f, 52.0f, 118.0f, 42.0f,
                   0.14f, 0.44f, 0.74f,
                   0.06f, 0.20f, 0.40f,
                   "CHARACTERS", GLUT_BITMAP_HELVETICA_12);
    }
 
    // ── CHARACTER SELECT ─────────────────────────────────────────────────────
    else if (state == GAME_STATE_CHARACTER_SELECT)
    {
        // Dark vignette overlay
        fillRect(0, 0, windowWidth, windowHeight, 0.02f, 0.05f, 0.12f, 0.82f);
 
        // ── Title ────────────────────────────────────────────────────────────
        {
            std::string hdr = "CHOOSE  YOUR  HOPPER";
            float tw = 0; for (char c : hdr) tw += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, c);
            // Banner under title
            fillRect(cx - tw/2 - 20, windowHeight - 72, cx + tw/2 + 20, windowHeight - 32, 0.05f, 0.12f, 0.28f, 0.85f);
            fillRect(cx - tw/2 - 20, windowHeight - 74, cx + tw/2 + 20, windowHeight - 70, 1.0f, 0.82f, 0.0f);
            fillRect(cx - tw/2 - 20, windowHeight - 34, cx + tw/2 + 20, windowHeight - 30, 1.0f, 0.82f, 0.0f);
            drawCentered(cx, windowHeight - 57, hdr, GLUT_BITMAP_TIMES_ROMAN_24, 1.0f, 1.0f, 1.0f, true);
        }
 
        // ── Character name (animated gold pulse) ─────────────────────────────
        const char* charNames[] = { "CHICKEN", "FROG", "DINO", "CAT", "DOG" };
        float nameAlpha = 0.65f + 0.35f * std::abs(std::sin(timeMs * 0.0038f));
        {
            std::string nm = charNames[selectedCharacterIndex];
            float nw = 0; for (char c : nm) nw += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, c);
            glColor4f(1.0f, 0.85f, 0.0f, nameAlpha);
            glRasterPos2f(cx - nw/2, windowHeight - 108.0f);
            for (char c : nm) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        }
 
        // ── Carousel cards ───────────────────────────────────────────────────
        float carouselY = cy + 30.0f;
        // slot layout: offsets & sizes for 5 visible cards (−2 … +2)
        float offsets[5] = { -310.0f, -170.0f,   0.0f, 170.0f, 310.0f };
        float sizes[5]   = {   48.0f,   68.0f, 105.0f,  68.0f,  48.0f };
        int   numChars   = 5;
 
        for (int slot = 0; slot < 5; slot++) {
            int ci = ((selectedCharacterIndex - 2 + slot) % numChars + numChars) % numChars;
            bool isCenter = (slot == 2);
            drawCharIcon(cx + offsets[slot], carouselY, sizes[slot], ci, isCenter);
        }
 
        // ── Navigation arrows ─────────────────────────────────────────────────
        float arrowR = 38.0f;
        float lax = 52.0f, rax = windowWidth - 52.0f, ay = carouselY;
 
        // Left arrow
        drawCircle(lax, ay, arrowR + 3, 0.0f, 0.0f, 0.0f, 0.5f);   // shadow
        drawCircle(lax, ay, arrowR,     0.12f, 0.32f, 0.62f, 0.90f);
        drawCircle(lax, ay, arrowR - 4, 0.18f, 0.44f, 0.80f, 0.70f); // sheen ring
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(lax + 14.0f, ay - 18.0f);
        glVertex2f(lax - 16.0f, ay);
        glVertex2f(lax + 14.0f, ay + 18.0f);
        glEnd();
 
        // Right arrow
        drawCircle(rax, ay, arrowR + 3, 0.0f, 0.0f, 0.0f, 0.5f);
        drawCircle(rax, ay, arrowR,     0.12f, 0.32f, 0.62f, 0.90f);
        drawCircle(rax, ay, arrowR - 4, 0.18f, 0.44f, 0.80f, 0.70f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(rax - 14.0f, ay - 18.0f);
        glVertex2f(rax + 16.0f, ay);
        glVertex2f(rax - 14.0f, ay + 18.0f);
        glEnd();
 
        // Dot indicators (5 dots)
        float dotY = carouselY - 100.0f;
        for (int d = 0; d < numChars; d++) {
            float dotX = cx + (d - 2) * 22.0f;
            bool active = (d == selectedCharacterIndex);
            drawCircle(dotX, dotY, active ? 7.0f : 4.5f,
                       active ? 1.0f : 0.5f,
                       active ? 0.84f : 0.5f,
                       active ? 0.0f : 0.5f, 1.0f, 12);
        }
 
        // ── SELECT button ────────────────────────────────────────────────────
        drawButton(cx, cy - 148.0f, 200.0f, 56.0f,
                   0.08f, 0.74f, 0.08f,
                   0.04f, 0.38f, 0.04f,
                   "SELECT!", GLUT_BITMAP_TIMES_ROMAN_24);
 
        // ── Back button ──────────────────────────────────────────────────────
        drawButton(55.0f, windowHeight - 50.0f, 84.0f, 34.0f,
                   0.35f, 0.35f, 0.45f,
                   0.18f, 0.18f, 0.24f,
                   "< BACK", GLUT_BITMAP_HELVETICA_12);
    }
 
    // ── PLAYING ──────────────────────────────────────────────────────────────
    else if (state == GAME_STATE_PLAYING)
    {
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
    }
 
    // ── START SCREEN ─────────────────────────────────────────────────────────
    else if (state == GAME_STATE_START_SCREEN)
    {
        if ((timeMs / 300) % 2 == 0) {
            drawCentered(cx, cy + 100.0f, "TAP EGG TO HATCH!",
                         GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, true);
        }
    }
 
    // ── GAME OVER ────────────────────────────────────────────────────────────
    else if (state == GAME_STATE_GAME_OVER)
    {
        // Full dark vignette
        fillRect(0, 0, windowWidth, windowHeight, 0.0f, 0.0f, 0.0f, 0.52f);
 
        // Panel
        float pW = 340.0f, pH = 230.0f, pY = cy + 22.0f;
        // Panel shadow
        fillRect(cx - pW/2 + 7, pY - pH/2 - 7, cx + pW/2 + 7, pY + pH/2 - 7, 0, 0, 0, 0.75f);
        // Red glow border
        fillRect(cx - pW/2 - 4, pY - pH/2 - 4, cx + pW/2 + 4, pY + pH/2 + 4, 0.65f, 0.08f, 0.08f, 0.92f);
        // Panel body
        fillRect(cx - pW/2, pY - pH/2, cx + pW/2, pY + pH/2, 0.04f, 0.04f, 0.11f, 0.97f);
        // Top inner sheen
        fillRect(cx - pW/2 + 4, pY + pH/2 - 12, cx + pW/2 - 4, pY + pH/2 - 4, 0.45f, 0.08f, 0.08f, 0.55f);
 
        // GAME OVER heading
        drawCentered(cx, pY + 88.0f, "GAME  OVER", GLUT_BITMAP_TIMES_ROMAN_24, 1.0f, 0.22f, 0.22f, true);
 
        // Divider
        fillRect(cx - 130, pY + 62, cx + 130, pY + 65, 0.55f, 0.10f, 0.10f, 0.80f);
 
        // Score row
        std::stringstream sText; sText << "SCORE      " << score;
        drawCentered(cx, pY + 36.0f, sText.str(), GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, true);
 
        // Coins row
        std::stringstream cText; cText << "+" << coinScore << "  COINS";
        drawCentered(cx, pY + 6.0f, cText.str(), GLUT_BITMAP_HELVETICA_18, 1.0f, 0.85f, 0.0f, true);
 
        // New best banner
        if (score > 0 && score == highScore) {
            float pulse = 0.7f + 0.3f * std::abs(std::sin(timeMs * 0.005f));
            fillRect(cx - 110, pY - 22, cx + 110, pY - 4, 0.6f * pulse, 0.5f * pulse, 0.0f, 0.60f);
            drawCentered(cx, pY - 18.0f, "* NEW BEST! *", GLUT_BITMAP_HELVETICA_12,
                         1.0f * pulse, 0.9f * pulse, 0.0f, false);
        }
 
        // TRY AGAIN button
        drawButton(cx, cy - 98.0f, 230.0f, 62.0f,
                   0.08f, 0.72f, 0.08f,
                   0.04f, 0.36f, 0.04f,
                   "TRY  AGAIN", GLUT_BITMAP_TIMES_ROMAN_24);
    }
 
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
void Game::onMouseClick(int button, int clickState, int x, int y)
{
    if (clickState != 0) return;
 
    int invertedY = windowHeight - y;
    float cx = windowWidth / 2.0f;
    float cy = windowHeight / 2.0f;
 
    if (state == GAME_STATE_MAIN_MENU)
    {
        // CHARACTERS button – bottom-left (cx=72, cy=52, w=118, h=42)
        if (x >= 13 && x <= 131 && invertedY >= 31 && invertedY <= 73) {
            state = GAME_STATE_CHARACTER_SELECT;
        }
        else {
            // Everything else starts the game
            state = GAME_STATE_START_SCREEN;
            eggClicks = 0;
        }
    }
    else if (state == GAME_STATE_CHARACTER_SELECT)
    {
        float carouselY  = cy + 30.0f;
        float arrowR     = 38.0f;
        float lax        = 52.0f;
        float rax        = windowWidth - 52.0f;
        int   numChars   = 5;
 
        // Left arrow
        float dlx = x - lax, dly = invertedY - carouselY;
        if (dlx*dlx + dly*dly <= arrowR*arrowR) {
            selectedCharacterIndex = (selectedCharacterIndex - 1 + numChars) % numChars;
        }
        // Right arrow
        else {
            float drx = x - rax, dry = invertedY - carouselY;
            if (drx*drx + dry*dry <= arrowR*arrowR) {
                selectedCharacterIndex = (selectedCharacterIndex + 1) % numChars;
            }
            // SELECT button (cx, cy-148, w=200, h=56)
            else if (x > cx - 100 && x < cx + 100 && invertedY > cy - 176 && invertedY < cy - 120) {
                decltype(MODEL_CHICKEN) models[] = { MODEL_CHICKEN, MODEL_FROG, MODEL_DINO, MODEL_CAT, MODEL_DOG };                player.setModel(models[selectedCharacterIndex]);
                state = GAME_STATE_MAIN_MENU;
            }
            // BACK button (cx=55, cy=windowHeight-50, w=84, h=34)
            else if (x >= 13 && x <= 97 &&
                     invertedY >= windowHeight - 67 && invertedY <= windowHeight - 33) {
                state = GAME_STATE_MAIN_MENU;
            }
        }
    }
    else if (state == GAME_STATE_START_SCREEN)
    {
        eggClicks++;
        lastClickTime = glutGet(GLUT_ELAPSED_TIME);
        if (eggClicks >= Config::MAX_EGG_CLICKS)
        {
            state = GAME_STATE_PLAYING;
            player.move(0.0f, 0.0f);
        }
    }
    else if (state == GAME_STATE_GAME_OVER)
    {
        // TRY AGAIN button (cx, cy-98, w=230, h=62)
        float btnW = 230.0f, btnH = 62.0f;
        float retryY = cy - 98.0f;
        if (x > cx - btnW/2 && x < cx + btnW/2 &&
            invertedY > retryY - btnH/2 && invertedY < retryY + btnH/2) {
            resetGame();
            state = GAME_STATE_MAIN_MENU;
        }
    }
}