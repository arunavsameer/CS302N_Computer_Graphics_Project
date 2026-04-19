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
      deathPosition(0.0f), hasWaterDeath(false), hasStreamDeath(false),
      lastClickTime(0)
{
    srand(static_cast<unsigned>(time(nullptr)));
}

void Game::resetGame()
{
    state = GAME_STATE_START_SCREEN;
    eggClicks = 0;
    coinScore = 0;
    score = 0;
    hasWaterDeath = false;
    hasStreamDeath = false;
    deathPosition = glm::vec3(0.0f);

    player.reset();
    camera.resetToDefault();

    lanes.clear();
    currentGenerationZ = 5.0f * Config::CELL_SIZE;

    for (int i = 0; i < Config::INITIAL_SAFE_ZONE_LENGTH; i++)
    {
        lanes.push_back(Lane(currentGenerationZ, LANE_GRASS, safePathColumn));
        currentGenerationZ -= Config::CELL_SIZE;
    }
    for (int i = 0; i < 5; i++)
        generateLaneBlock();

    cameraTrackZ = player.getPosition().z;
    startZ = player.getPosition().z;
}

void Game::initialize()
{
    renderer.initialize();

    for (int i = 0; i < Config::INITIAL_SAFE_ZONE_LENGTH; i++)
    {
        lanes.push_back(Lane(currentGenerationZ, LANE_GRASS, safePathColumn));
        currentGenerationZ -= Config::CELL_SIZE;
    }
    for (int i = 0; i < 5; i++)
        generateLaneBlock();

    cameraTrackZ = player.getPosition().z;
    startZ = player.getPosition().z;
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

    if (state == GAME_STATE_START_SCREEN)
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
    // score = coinScore;
    int lanesMoved = static_cast<int>(std::round((startZ - player.getBasePosition().z) / Config::CELL_SIZE));
    if (lanesMoved > score) {
        score = lanesMoved;
    }
}

void Game::checkCollisions(float deltaTime)
{
    glm::vec3 playerPos = player.getPosition();
    glm::vec3 playerSize = player.getSize();

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

            bool hitX = std::abs(player.getBasePosition().x - obs.getPosition().x) < (player.getSize().x + obs.getSize().x) * 0.5f * Config::HITBOX_PADDING;
            bool hitZ = std::abs(playerPos.z - obs.getPosition().z) < Config::CELL_SIZE * 0.9f;

            if (hitX && hitZ)
            {
                player.setDead(true);
                deathPosition = playerPos;
                hasWaterDeath = false;
                state = GAME_STATE_GAME_OVER;
                return;
            }
        }
    }

    for (auto &obs : currentLane->getObstacles())
    {
        if (!obs.getIsActive())
            continue;

        bool isColliding = false;
        if (obs.getType() == OBSTACLE_LOG || obs.getType() == OBSTACLE_LILYPAD)
        {
            isColliding = Collision::checkAABB(
                player.getBasePosition(), playerSize,
                obs.getPosition(), obs.getSize());
        }
        else
        {
            isColliding = Collision::checkAABB(
                playerPos, playerSize,
                obs.getPosition(), obs.getSize());
        }

        if (isColliding)
        {
            if (obs.getType() == OBSTACLE_CAR || obs.getType() == OBSTACLE_TRAIN)
            {
                player.setDead(true);
                deathPosition = playerPos;
                hasWaterDeath = false;
                state = GAME_STATE_GAME_OVER;
                return;
            }
            else if (obs.getType() == OBSTACLE_LOG)
            {
                onLog = true;
                obs.setSinking(true);
                player.applyLogVelocity(obs.getSpeed(), deltaTime);

                float logX = obs.getPosition().x;
                if (std::abs(logX) > Config::LOG_STREAM_TRIGGER_X)
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
                    return;
                }
            }
            else if (obs.getType() == OBSTACLE_LILYPAD)
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
        }
    }

    if ((currentLane->getType() == LANE_RIVER || currentLane->getType() == LANE_LILYPAD) && !player.getIsJumping() && !onLog && !onLilypad)
    {
        const float waterSurface = -Config::CELL_SIZE * 0.1f;
        player.triggerWaterDeath(waterSurface);
        deathPosition = playerPos;
        hasWaterDeath = true;
        hasStreamDeath = false;
        state = GAME_STATE_GAME_OVER;
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

void Game::render()
{
    renderer.prepareFrame();
    camera.apply();

    renderWorldBoundaries();

    for (auto &lane : lanes)
        lane.render(renderer);

    if (state == GAME_STATE_START_SCREEN)
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
    else
    {
        int currentTime = glutGet(GLUT_ELAPSED_TIME);
        float timeSinceStart = (currentTime - lastClickTime) / 1000.0f;
        float spawnDuration = 0.4f;

        if (state == GAME_STATE_PLAYING && timeSinceStart < spawnDuration)
        {
            float t = timeSinceStart / spawnDuration;
            float t1 = t - 1.0f;
            float scale = t1 * t1 * (2.5f * t1 + 1.5f) + 1.0f;
            if (scale < 0.0f)
                scale = 0.0f;

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

    camera.renderOverlay(windowWidth, windowHeight);
    renderUIOverlay();
}

void Game::onKeyPress(unsigned char key)
{
    if (key == 'n' || key == 'N')
    {
        nightMode = !nightMode;
        renderer.setNightMode(nightMode);
        return;
    }

    // --- NEW: Swap character on the start screen ---
    if (state == GAME_STATE_START_SCREEN) {
        if (key == '1') player.setModel(MODEL_CHICKEN);
        if (key == '2') player.setModel(MODEL_FROG);
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

    if (state == GAME_STATE_PLAYING || state == GAME_STATE_GAME_OVER)
    {
        glColor3f(1, 1, 1);
        std::stringstream ss;
        ss << "Score: " << score << "   Coins: " << coinScore;
        glRasterPos2f(20, windowHeight - 40);
        for (char c : ss.str())
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    if (state == GAME_STATE_START_SCREEN)
    {
        glColor3f(1, 1, 1);
        std::string msg = "Click Egg to Hatch! (Press 1: Chicken, 2: Frog)";
        glRasterPos2f(windowWidth / 2.0f - 160.0f, windowHeight / 2.0f + 100.0f);
        for (char c : msg)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    else if (state == GAME_STATE_GAME_OVER)
    {
        float boxW = 140.0f, boxH = 60.0f;
        float cx = windowWidth / 2.0f;
        float cy = 80.0f;

        glColor3f(1.0f, 0.6f, 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(cx - boxW / 2, cy - boxH / 2);
        glVertex2f(cx + boxW / 2, cy - boxH / 2);
        glVertex2f(cx + boxW / 2, cy + boxH / 2);
        glVertex2f(cx - boxW / 2, cy + boxH / 2);
        glEnd();
        glLineWidth(4.0f);
        glColor3f(0.1f, 0.4f, 0.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx - boxW / 2, cy - boxH / 2);
        glVertex2f(cx + boxW / 2, cy - boxH / 2);
        glVertex2f(cx + boxW / 2, cy + boxH / 2);
        glVertex2f(cx - boxW / 2, cy + boxH / 2);
        glEnd();
        glLineWidth(1.0f);
        glColor3f(1, 1, 1);
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

void Game::onMouseClick(int button, int clickState, int x, int y)
{
    if (clickState != 0)
        return;

    if (state == GAME_STATE_START_SCREEN)
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
        int invertedY = windowHeight - y;
        float boxW = 140.0f, boxH = 60.0f;
        float cx = windowWidth / 2.0f;
        float cy = 80.0f;

        if (x > cx - boxW / 2 && x < cx + boxW / 2 &&
            invertedY > cy - boxH / 2 && invertedY < cy + boxH / 2)
            resetGame();
    }
}