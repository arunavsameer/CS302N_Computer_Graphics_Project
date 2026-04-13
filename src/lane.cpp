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
        // Start well off screen so the player always has a moment before
        // the first pass; further randomisation is in the respawn timer.
        float startX = dir * -(35.0f + static_cast<float>(rand() % 20));

        obstacles.push_back(Obstacle(
            glm::vec3(startX, Config::CELL_SIZE * 0.6f, zPosition),
            speed, OBSTACLE_TRAIN));

        // ── Signal posts every SIGNAL_SPACING units across the track ─────────
        // Each post gets a small random X jitter so they look naturally placed.
        // Posts sit at the near edge of the lane (z + 0.42) so they are
        // visually beside the rails and the train body clears them cleanly.
        // Blocking uses only the X axis (player snaps to exact lane Z).
        constexpr float SIGNAL_SPACING = 8.0f;   // base interval between posts
        constexpr float SIGNAL_RANGE   = 20.0f;  // ±range from centre
        constexpr float SIGNAL_Z_SIDE  = 0.42f;  // visual offset to lane near-edge
        constexpr float SIGNAL_JITTER  = 1.4f;   // max random X nudge per post

        for (float sx = -SIGNAL_RANGE; sx <= SIGNAL_RANGE + 0.01f; sx += SIGNAL_SPACING) {
            // Small random X jitter so posts aren't perfectly evenly spaced
            float jitter = SIGNAL_JITTER * (static_cast<float>(rand()) /
                           static_cast<float>(RAND_MAX) * 2.0f - 1.0f);
            float finalX = sx + jitter;
            if (finalX < -SIGNAL_RANGE) finalX = -SIGNAL_RANGE;
            if (finalX >  SIGNAL_RANGE) finalX =  SIGNAL_RANGE;

            // Don't place a post on the safe path column
            float safeWorldX = safePathColumn * Config::CELL_SIZE;
            if (std::abs(finalX - safeWorldX) < Config::CELL_SIZE * 0.7f)
                continue;

            SignalPost sp;
            sp.position = glm::vec3(finalX, 0.0f, zPosition + SIGNAL_Z_SIDE);
            signalPosts.push_back(sp);
        }
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
    else if (type == LANE_LILYPAD) {
        // 1. Guaranteed lilypad on the safe path so the player can cross
        float safeX = safePathColumn * Config::CELL_SIZE;
        obstacles.push_back(Obstacle(
            glm::vec3(safeX, Config::LILYPAD_Y, zPosition), 
            0.0f, OBSTACLE_LILYPAD));

        // 2. Randomly spawn lilypads to the LEFT
        float currX = safeX - (Config::LILYPAD_GAP_MIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (Config::LILYPAD_GAP_MAX - Config::LILYPAD_GAP_MIN)));
        while (currX > -15.0f) {
            obstacles.push_back(Obstacle(
                glm::vec3(currX, Config::LILYPAD_Y, zPosition), 
                0.0f, OBSTACLE_LILYPAD));
            currX -= (Config::LILYPAD_GAP_MIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (Config::LILYPAD_GAP_MAX - Config::LILYPAD_GAP_MIN)));
        }

        // 3. Randomly spawn lilypads to the RIGHT
        currX = safeX + (Config::LILYPAD_GAP_MIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (Config::LILYPAD_GAP_MAX - Config::LILYPAD_GAP_MIN)));
        while (currX < 15.0f) {
            obstacles.push_back(Obstacle(
                glm::vec3(currX, Config::LILYPAD_Y, zPosition), 
                0.0f, OBSTACLE_LILYPAD));
            currX += (Config::LILYPAD_GAP_MIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (Config::LILYPAD_GAP_MAX - Config::LILYPAD_GAP_MIN)));
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
static void drawSignalPost(Renderer& renderer,
                           glm::vec3 base,
                           bool lightRed,
                           bool lightGreen)
{
    // ── Pole ─────────────────────────────────────────────────────────────────
    renderer.drawCube(base + glm::vec3(0.0f, 0.55f, 0.0f),
                      glm::vec3(0.09f, 1.10f, 0.09f),
                      glm::vec3(0.40f, 0.40f, 0.42f));

    // ── Signal housing (dark box) ─────────────────────────────────────────
    renderer.drawCube(base + glm::vec3(0.0f, 1.20f, 0.0f),
                      glm::vec3(0.34f, 0.52f, 0.22f),
                      glm::vec3(0.13f, 0.13f, 0.15f));

    // ── Red light (top) ──────────────────────────────────────────────────
    glm::vec3 redCol = lightRed
        ? glm::vec3(1.00f, 0.08f, 0.08f)
        : glm::vec3(0.28f, 0.04f, 0.04f);
    renderer.drawCube(base + glm::vec3(0.0f, 1.35f, 0.0f),
                      glm::vec3(0.20f, 0.20f, 0.24f), redCol);

    // ── Green light (bottom) ─────────────────────────────────────────────
    glm::vec3 greenCol = lightGreen
        ? glm::vec3(0.10f, 1.00f, 0.12f)
        : glm::vec3(0.04f, 0.28f, 0.05f);
    renderer.drawCube(base + glm::vec3(0.0f, 1.04f, 0.0f),
                      glm::vec3(0.20f, 0.20f, 0.24f), greenCol);

    // ── Crossbuck arm ────────────────────────────────────────────────────
    renderer.drawCube(base + glm::vec3(0.0f, 0.86f, 0.0f),
                      glm::vec3(0.56f, 0.09f, 0.09f),
                      glm::vec3(0.92f, 0.92f, 0.92f));

    // ── RR bump details on crossbuck ─────────────────────────────────────
    renderer.drawCube(base + glm::vec3(-0.15f, 0.86f, 0.0f),
                      glm::vec3(0.08f, 0.18f, 0.08f),
                      glm::vec3(0.92f, 0.92f, 0.92f));
    renderer.drawCube(base + glm::vec3( 0.15f, 0.86f, 0.0f),
                      glm::vec3(0.08f, 0.18f, 0.08f),
                      glm::vec3(0.92f, 0.92f, 0.92f));
}


void Lane::render(Renderer& renderer) {

    // ── RIVER / LILYPAD: animated water ──────────────────────────────────────
    if (type == LANE_RIVER || type == LANE_LILYPAD) {
        const float yPos = -Config::CELL_SIZE * 0.2f;
        renderer.drawAnimatedWater(
            glm::vec3(0.0f, yPos, zPosition),
            glm::vec3(30.0f, Config::CELL_SIZE * 0.2f, Config::CELL_SIZE));

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
                renderer.drawCube(d.position + glm::vec3(0, -0.2f, 0),
                                  glm::vec3(0.25f, 0.6f * d.scale, 0.25f),
                                  glm::vec3(0.55f, 0.27f, 0.07f));
                renderer.drawCube(d.position + glm::vec3(0, 0.3f * d.scale, 0),
                                  glm::vec3(0.7f, 0.7f, 0.7f), d.color);
                renderer.drawCube(d.position + glm::vec3(0, 0.7f * d.scale, 0),
                                  glm::vec3(0.5f, 0.5f, 0.5f), d.color);
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
                else if (absX < 25.0f) trainApproach = true;
            }
        }

        // Flash at ~3 Hz when train is approaching
        float t       = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;
        bool  flashOn = ((int)(t * 6.0f) % 2) == 0;

        bool lightRed   = trainPassing || (trainApproach && flashOn);
        bool lightGreen = !trainPassing && !trainApproach;

        // Draw every signal post stored in signalPosts
        for (const auto& sp : signalPosts) {
            drawSignalPost(renderer, sp.position, lightRed, lightGreen);
        }
    }
}