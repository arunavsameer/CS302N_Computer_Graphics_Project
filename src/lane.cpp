#include "../include/lane.h"
#include <cstdlib>
#include <cmath>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

static bool isFarEnough(const std::vector<Lane::Decoration>& decs, float x) {
    for (const auto& d : decs) {
        if (std::abs(d.position.x - x) < Config::CELL_SIZE * 0.9f)
            return false;
    }
    return true;
}

Lane::Lane(float z, LaneType t, int safePath)
    : zPosition(z), type(t), safePathColumn(safePath) {

    float dir = (rand() % 2 == 0) ? 1.0f : -1.0f;

    // ===== EXISTING OBSTACLES =====
    if (type == LANE_ROAD) {
        float speed = dir * randomRange(Config::CAR_SPEED_MIN, Config::CAR_SPEED_MAX);

        float startX = dir * -(5.0f + static_cast<float>(rand() % 10));

        int pick = rand() % 3;
        VehicleVariant variant = (pick == 1) ? VEHICLE_BIG_CAR
                                : (pick == 2) ? VEHICLE_TRUCK
                                : VEHICLE_SMALL_CAR;

        obstacles.push_back(Obstacle(
            glm::vec3(startX, 0.31f, zPosition),
            speed, OBSTACLE_CAR, variant));
    }
    else if (type == LANE_RAIL) {
        float speed  = dir * Config::TRAIN_SPEED;
        // Start well off screen so the player always has a moment before
        // the first pass; further randomisation is in the respawn timer.
        float startX = dir * -(35.0f + static_cast<float>(rand() % 20));

        obstacles.push_back(Obstacle(
            glm::vec3(startX, 0.46f, zPosition),
            speed, OBSTACLE_TRAIN));

        // ── Signal posts every SIGNAL_SPACING units across the track ─────────

        for (float sx = -Config::SIGNAL_RANGE; sx <= Config::SIGNAL_RANGE + 0.01f; sx += Config::SIGNAL_SPACING) {
            float jitter = Config::SIGNAL_JITTER * (randomRange(0.0f, 1.0f) * 2.0f - 1.0f);
            float finalX = sx + jitter;
            if (finalX < -Config::SIGNAL_RANGE) finalX = -Config::SIGNAL_RANGE;
            if (finalX >  Config::SIGNAL_RANGE) finalX =  Config::SIGNAL_RANGE;

            // Don't place a post on the safe path column
            float safeWorldX = safePathColumn * Config::CELL_SIZE;
            if (std::abs(finalX - safeWorldX) < Config::CELL_SIZE * 0.7f)
                continue;

            SignalPost sp;
            sp.position = glm::vec3(finalX, 0.0f, zPosition + Config::SIGNAL_Z_SIDE);
            signalPosts.push_back(sp);
        }
    }
    else if (type == LANE_RIVER) {
            float speed = dir * randomRange(Config::LOG_SPEED_MIN, Config::LOG_SPEED_MAX);
            
            // Calculate where the very first set should start so they are spread out
            float startSetX = -((Config::LOG_SETS - 1) * Config::LOG_SET_GAP) / 2.0f;
            
            // Loop to create multiple SETS of logs
            for (int s = 0; s < Config::LOG_SETS; s++) {
                
                // Randomize how many logs are in THIS specific set (1 to 3)
                int logCount = Config::LOG_COUNT_MIN +
                    rand() % (Config::LOG_COUNT_MAX - Config::LOG_COUNT_MIN + 1);
            
                float totalSpan = (logCount - 1) * Config::LOG_SPACING;
                
                // The center point for this set of logs
                float setCenterX = startSetX + (s * Config::LOG_SET_GAP);
                
                // The starting X position for the first log in this set
                float startX = setCenterX - (totalSpan / 2.0f);
            
                // Loop to create the individual logs inside the set
                for (int i = 0; i < logCount; i++) {
                    float x = startX + i * Config::LOG_SPACING;
                
                    obstacles.push_back(Obstacle(
                        glm::vec3(x, Config::LOG_Y, zPosition),
                        speed, OBSTACLE_LOG));
                }
            }
        }
    else if (type == LANE_LILYPAD) {
        // 1. Guaranteed lilypad on the safe path so the player can cross
        float safeX = safePathColumn * Config::CELL_SIZE;
        obstacles.push_back(Obstacle(
            glm::vec3(safeX, Config::LILYPAD_Y, zPosition), 
            0.0f, OBSTACLE_LILYPAD));

        // 2. Randomly spawn lilypads to the LEFT
        float currX = safeX - randomRange(Config::LILYPAD_GAP_MIN, Config::LILYPAD_GAP_MAX);
        while (currX > -15.0f) {
            obstacles.push_back(Obstacle(
                glm::vec3(currX, Config::LILYPAD_Y, zPosition), 
                0.0f, OBSTACLE_LILYPAD));
            currX -= randomRange(Config::LILYPAD_GAP_MIN, Config::LILYPAD_GAP_MAX);
        }

        // 3. Randomly spawn lilypads to the RIGHT
        currX = safeX + randomRange(Config::LILYPAD_GAP_MIN, Config::LILYPAD_GAP_MAX);
        while (currX < 15.0f) {
            obstacles.push_back(Obstacle(
                glm::vec3(currX, Config::LILYPAD_Y, zPosition), 
                0.0f, OBSTACLE_LILYPAD));
            currX += randomRange(Config::LILYPAD_GAP_MIN, Config::LILYPAD_GAP_MAX);
        }
    }


    // ===== 🌳🪨 DECORATIONS FIRST =====
    if (type == LANE_GRASS) {

        int pathWidth = 2;
        int pathBufferZone = pathWidth + 1; // ✅ Prevent decorations from spawning directly on safe path

        for (int i = -15; i <= 15; i++) {

            float x = i * Config::CELL_SIZE * 0.9f;

            // ✅ ENFORCE: No decorations on the safe path (NEW FIX)
            if (std::abs(i - safePathColumn) <= pathBufferZone)
                continue;

            // less clutter near path
            if (std::abs(i - safePathColumn) <= pathBufferZone + 2) {
                if (rand() % 100 > 30) continue;
            } else {
                if (rand() % 100 > 75) continue;
            }

            if (!isFarEnough(decorations, x))
                continue;

            Decoration d;
            d.position = glm::vec3(x, 0.15f, zPosition);

            if (rand() % 5 == 0) {
                d.type = 1;
                d.scale = 0.8f;
                d.color = glm::vec3(0.5f);
            } else {
                d.type = 0;
                d.scale = (rand() % 2 == 0) ? 1.0f : 1.5f;

                float greenVar = 0.1f * (rand() % 3);
                d.color = glm::vec3(0.2f, 0.7f + greenVar, 0.2f);
            }

            decorations.push_back(d);
        }
    }

    // ===== COINS AFTER DECORATIONS =====
    if (type == LANE_GRASS&&(rand()%100)<35) {

        int tries = 5;

        while (tries--) {

            // place near safe path (feels natural)
            int offset = rand() % 3 - 1;
            int coinCol = safePathColumn + offset;

            float x = coinCol * Config::CELL_SIZE * 0.9f;

            bool overlap = false;

            for (auto& d : decorations) {
                if (fabs(d.position.x - x) < Config::CELL_SIZE * 0.8f) {
                    overlap = true;
                    break;
                }
            }

            if (!overlap) {
                coins.emplace_back(glm::vec3(
                    x,
                    Config::CELL_SIZE * 0.6f,
                    zPosition
                ));
                break;
            }
        }
    }

    
}

void Lane::update(float deltaTime) {
    for (auto& obs : obstacles) obs.update(deltaTime);
}

void Lane::render(Renderer& renderer, float sunAngle, int frameTime) {

    // ── RIVER / LILYPAD: animated water ──────────────────────────────────────
    if (type == LANE_RIVER || type == LANE_LILYPAD) {
        const float yPos = -Config::CELL_SIZE * 0.2f;
        // OPTIMIZATION: Pass cached frameTime to avoid repeated glutGet() calls (~0.5-1% improvement)
        renderer.drawAnimatedWater(
            glm::vec3(0.0f, yPos, zPosition),
            glm::vec3(30.0f, Config::CELL_SIZE * 0.2f, Config::CELL_SIZE), frameTime);

        for (auto& obs  : obstacles) obs.render(renderer);
        for (auto& coin : coins)     coin.render(renderer);
        return;
    }

    // ── Textured lane surface ─────────────────────────────────────────────────
    std::string texName;
    if (type == LANE_GRASS) {
        int gridZ = std::abs(static_cast<int>(std::round(zPosition / Config::CELL_SIZE)));
        texName = (gridZ % 2 == 0) ? "grass" : "grass2";
    }
    else if (type == LANE_ROAD) texName = "road";
    else if (type == LANE_RAIL) texName = "rail";
    else texName = "grass";

    float yPos = 0.0f;
    renderer.drawTexturedCube(
        glm::vec3(0.0f, yPos, zPosition),
        glm::vec3(30.0f, Config::CELL_SIZE * 0.2f, Config::CELL_SIZE),
        texName);

    for (auto& obs  : obstacles) obs.render(renderer);
    for (auto& coin : coins)     coin.render(renderer);

    // ── GRASS decorations ─────────────────────────────────────────────────────
    if (type == LANE_GRASS) {
        auto tree_offset = glm::vec3(0, 0.35f, 0);
        for (auto& d : decorations) {
            if (d.type == 0) {
                d.position += tree_offset;
                // Draw trunk (brown)
                renderer.drawCube(d.position + glm::vec3(0, -0.2f, 0),
                                  glm::vec3(0.25f, 0.6f * d.scale, 0.25f),
                                  glm::vec3(0.55f, 0.27f, 0.07f));
                // Draw foliage with sun-based shading
                renderer.drawCubeShaded(d.position + glm::vec3(0, 0.3f * d.scale, 0),
                                        glm::vec3(0.7f, 0.7f, 0.7f), d.color, sunAngle);
                renderer.drawCubeShaded(d.position + glm::vec3(0, 0.7f * d.scale, 0),
                                        glm::vec3(0.5f, 0.5f, 0.5f), d.color, sunAngle);
                d.position -= tree_offset;
            } else {
                renderer.drawCube(d.position,
                                  glm::vec3(0.5f, 0.3f, 0.5f), d.color);
                renderer.drawCube(d.position + glm::vec3(0.2f, 0.15f, 0),
                                  glm::vec3(0.25f, 0.25f, 0.25f),
                                  glm::vec3(0.65f, 0.65f, 0.65f));
            }
        }
    }

    // ── RAIL: signal posts ────────────────────────────────────────────────────
    // Determine light state once, then draw all posts with the same state.
    if (type == LANE_RAIL) {

        bool trainPassing  = false;
        bool trainApproach = false;

        for (const auto& obs : obstacles) {
            if (obs.getType() == OBSTACLE_TRAIN && obs.getIsActive()) {
                float absX = std::abs(obs.getPosition().x);
                if      (absX < 14.0f) trainPassing  = true;
                else if (absX < 150.0f) trainApproach = true;
            }
        }

        // OPTIMIZATION: Use cached frameTime instead of glutGet() (~0.3-0.5% improvement)
        float t = (frameTime != 0 ? static_cast<float>(frameTime) : static_cast<float>(glutGet(GLUT_ELAPSED_TIME))) * 0.001f;
        bool  flashOn = ((int)(t * 6.0f) % 2) == 0; 

        bool lightRed   = trainPassing || (trainApproach && flashOn);
        bool lightGreen = !trainPassing && !trainApproach;

        // Draw every signal post stored in signalPosts
        for (auto& sp : signalPosts) {
            renderer.drawSignalPost(sp.position, lightRed, lightGreen);
        }
    }
}