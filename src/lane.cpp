#include "../include/lane.h"
#include <cstdlib>
#include <cmath>

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
        float speed  = dir * (Config::CAR_SPEED_MIN +
            static_cast<float>(rand()) /
            (static_cast<float>(RAND_MAX) / (Config::CAR_SPEED_MAX - Config::CAR_SPEED_MIN)));

        float startX = dir * -(5.0f + static_cast<float>(rand() % 10));

        int pick = rand() % 3;
        VehicleVariant variant = (pick == 1) ? VEHICLE_BIG_CAR
                                : (pick == 2) ? VEHICLE_TRUCK
                                : VEHICLE_SMALL_CAR;

        obstacles.push_back(Obstacle(
            glm::vec3(startX, Config::CELL_SIZE * 0.6f, zPosition),
            speed, OBSTACLE_CAR, variant));
    }
    else if (type == LANE_RAIL) {
        float speed  = dir * Config::TRAIN_SPEED;
        float startX = dir * -(30.0f + static_cast<float>(rand() % 50));

        obstacles.push_back(Obstacle(
            glm::vec3(startX, Config::CELL_SIZE * 0.6f, zPosition),
            speed, OBSTACLE_TRAIN));
    }
    else if (type == LANE_RIVER) {
        float speed = dir * (Config::LOG_SPEED_MIN +
            static_cast<float>(rand()) /
            (static_cast<float>(RAND_MAX) / (Config::LOG_SPEED_MAX - Config::LOG_SPEED_MIN)));

        int logCount = Config::LOG_COUNT_MIN +
            rand() % (Config::LOG_COUNT_MAX - Config::LOG_COUNT_MIN + 1);

        float totalSpan = (logCount - 1) * Config::LOG_SPACING;
        float startX = -totalSpan / 2.0f;

        for (int i = 0; i < logCount; i++) {
            float x = startX + i * Config::LOG_SPACING;

            obstacles.push_back(Obstacle(
                glm::vec3(x, Config::LOG_Y, zPosition),
                speed, OBSTACLE_LOG));
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

    // ===== 🪙 COINS AFTER DECORATIONS =====
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

void Lane::render(Renderer& renderer) {

    // ── RIVER: fully procedural animated water (no texture needed) ───────────
    if (type == LANE_RIVER) {
        const float yPos = -Config::CELL_SIZE * 0.2f;
        renderer.drawAnimatedWater(
            glm::vec3(0.0f, yPos, zPosition),
            glm::vec3(30.0f, Config::CELL_SIZE * 0.2f, Config::CELL_SIZE));

        for (auto& obs  : obstacles) obs.render(renderer);
        for (auto& coin : coins)     coin.render(renderer);
        return;
    }

    // ── All other lane types: textured cube ──

    std::string texName;

    if (type == LANE_GRASS) {
        int gridZ = std::abs(static_cast<int>(std::round(zPosition / Config::CELL_SIZE)));
        texName = (gridZ % 2 == 0) ? "grass" : "grass2";
    }
    else if (type == LANE_ROAD) texName = "road";
    else if (type == LANE_RAIL) texName = "rail";
    else if (type == LANE_RIVER) texName = "river";
    else texName = "grass";

    float yPos = (type == LANE_RIVER) ? -Config::CELL_SIZE * 0.2f : 0.0f;

    renderer.drawTexturedCube(
        glm::vec3(0.0f, yPos, zPosition),
        glm::vec3(30.0f, Config::CELL_SIZE * 0.2f, Config::CELL_SIZE),
        texName);

    for (auto& obs : obstacles) obs.render(renderer);
    for (auto& coin : coins) coin.render(renderer);

    for (auto& d : decorations) {

        if (d.type == 0) {
            renderer.drawCube(
                d.position + glm::vec3(0, -0.2f, 0),
                glm::vec3(0.25f, 0.6f * d.scale, 0.25f),
                glm::vec3(0.55f, 0.27f, 0.07f)
            );

            renderer.drawCube(
                d.position + glm::vec3(0, 0.3f * d.scale, 0),
                glm::vec3(0.7f, 0.7f, 0.7f),
                d.color
            );

            renderer.drawCube(
                d.position + glm::vec3(0, 0.7f * d.scale, 0),
                glm::vec3(0.5f, 0.5f, 0.5f),
                d.color
            );
        }
        else {
            renderer.drawCube(
                d.position,
                glm::vec3(0.5f, 0.3f, 0.5f),
                d.color
            );

            renderer.drawCube(
                d.position + glm::vec3(0.2f, 0.15f, 0),
                glm::vec3(0.25f, 0.25f, 0.25f),
                glm::vec3(0.65f, 0.65f, 0.65f)
            );
        }
    }
}