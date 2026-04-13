#include "../include/obstacle.h"
#include <cstdlib>

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────
Obstacle::Obstacle(glm::vec3 startPos, float spd, ObstacleType t, VehicleVariant variant)
    : position(startPos), speed(spd), type(t), vehicleVariant(variant),
      isActive(true), sinkOffset(0.0f), isSinking(false),
      startPosition(startPos), respawnTimer(0.0f)
{
    if (type == OBSTACLE_CAR) {
        if      (vehicleVariant == VEHICLE_SMALL_CAR) size = glm::vec3(1.45f, 0.62f, 0.78f);
        else if (vehicleVariant == VEHICLE_BIG_CAR)   size = glm::vec3(1.95f, 0.72f, 0.88f);
        else                                           size = glm::vec3(3.05f, 0.92f, 1.00f);
    }
    else if (type == OBSTACLE_TRAIN) {
        size = glm::vec3(Config::TRAIN_LENGTH * Config::CELL_SIZE,
                         Config::CELL_SIZE * 1.0f,
                         Config::CELL_SIZE * 0.9f);
        // Randomise the gap between passes: 5–10 seconds
        respawnDelay = 5.0f + static_cast<float>(rand() % 6);
    }
    else if (type == OBSTACLE_LOG) {
        size = glm::vec3(Config::LOG_WIDTH, Config::LOG_HEIGHT, Config::LOG_DEPTH);
    }
    else if (type == OBSTACLE_LILYPAD) {
        size = glm::vec3(Config::LILYPAD_SIZE, Config::LILYPAD_HEIGHT, Config::LILYPAD_SIZE);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Update
// ─────────────────────────────────────────────────────────────────────────────
void Obstacle::update(float deltaTime) {

    // ── TRAIN: active → move; inactive → wait then respawn ──────────────────
    if (type == OBSTACLE_TRAIN) {
        if (isActive) {
            position.x += speed * deltaTime;
            // Left the screen entirely → start the respawn countdown
            if ((speed > 0 && position.x > 65.0f) || (speed < 0 && position.x < -65.0f)) {
                isActive     = false;
                respawnTimer = respawnDelay;
            }
        } else {
            respawnTimer -= deltaTime;
            if (respawnTimer <= 0.0f) {
                // Reset back to the original off-screen start position
                position = startPosition;
                isActive = true;
            }
        }
        return;   // early-out: train never sinks
    }

    if (!isActive) return;

    position.x += speed * deltaTime;

    // Cars / logs wrap around
    if (speed > 0 && position.x >  15.0f) position.x = -15.0f;
    if (speed < 0 && position.x < -15.0f) position.x =  15.0f;

    // ── Log sinking effect ──────────────────────────────────────────────────
    if (type == OBSTACLE_LOG) {
        float target = isSinking ? -Config::LOG_SINK_AMOUNT : 0.0f;
        sinkOffset += (target - sinkOffset) * Config::LOG_SINK_SPEED * deltaTime;
        isSinking    = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Render helpers
// ─────────────────────────────────────────────────────────────────────────────
static void drawWheels(Renderer& renderer,
                       glm::vec3 basePos,
                       float xFront, float xBack, float zSide,
                       float wheelSize)
{
    glm::vec3 wc(0.08f, 0.08f, 0.08f);
    glm::vec3 ws(wheelSize);
    renderer.drawCube(basePos + glm::vec3(xFront, 0.0f,  zSide), ws, wc);
    renderer.drawCube(basePos + glm::vec3(xBack,  0.0f,  zSide), ws, wc);
    renderer.drawCube(basePos + glm::vec3(xFront, 0.0f, -zSide), ws, wc);
    renderer.drawCube(basePos + glm::vec3(xBack,  0.0f, -zSide), ws, wc);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Render
// ─────────────────────────────────────────────────────────────────────────────
void Obstacle::render(Renderer& renderer) {
    if (!isActive) return;

    // ── LOG ──────────────────────────────────────────────────────────────────
    if (type == OBSTACLE_LOG) {
        glm::vec3 renderPos = position;
        renderPos.y += sinkOffset;
        renderer.drawTexturedCube(renderPos, size, "log");
        return;
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  TRAIN  (locomotive + 3 passenger cars)
    //  The full train is ~20 units long.  Segment centres are relative to the
    //  obstacle's position (centre of the bounding box).
    //
    //  dir = +1 → moving right, locomotive at the RIGHT (front)
    //        -1 → moving left,  locomotive at the LEFT  (front)
    // ─────────────────────────────────────────────────────────────────────────
    if (type == OBSTACLE_TRAIN) {
        const float dir = (speed >= 0.0f) ? 1.0f : -1.0f;
        const glm::vec3 p = position;

        // ── Colour palette ───────────────────────────────────────────────────
        glm::vec3 bodyBlue (0.18f, 0.45f, 0.82f);
        glm::vec3 roofWhite(0.92f, 0.92f, 0.96f);
        glm::vec3 darkWin  (0.08f, 0.09f, 0.12f);
        glm::vec3 wheelCol (0.08f, 0.08f, 0.08f);
        glm::vec3 stackCol (0.28f, 0.28f, 0.30f);
        glm::vec3 headlamp (1.00f, 0.96f, 0.70f);
        glm::vec3 redStripe(0.88f, 0.14f, 0.14f);
        glm::vec3 cowcatch (0.14f, 0.14f, 0.16f);
        glm::vec3 carBody  (0.74f, 0.78f, 0.83f);
        glm::vec3 steamCol (0.95f, 0.95f, 0.95f);

        // ── Segment centres ──────────────────────────────────────────────────
        // Train spans pos.x ± ~10 units (total ~20 u).
        // Loco 4.5 u wide | Car1 4.7 u | Car2 4.7 u | Car3 4.7 u
        // gaps of 0.3 u between each
        glm::vec3 loco(p.x + dir * 7.75f, p.y, p.z);
        glm::vec3 c1  (p.x + dir * 2.85f, p.y, p.z);
        glm::vec3 c2  (p.x - dir * 2.15f, p.y, p.z);
        glm::vec3 c3  (p.x - dir * 7.15f, p.y, p.z);

        // ════════════════════════════════════════════════════════════════════
        //  LOCOMOTIVE
        // ════════════════════════════════════════════════════════════════════

        // Lower body slab
        renderer.drawCube(loco,
                          glm::vec3(4.5f, 0.58f, 0.88f), bodyBlue);

        // Upper cab (rear 2/3 of loco)
        renderer.drawCube(loco + glm::vec3(-dir * 0.65f, 0.44f, 0.0f),
                          glm::vec3(3.0f, 0.36f, 0.82f), roofWhite);

        // Cab roof cap
        renderer.drawCube(loco + glm::vec3(-dir * 0.65f, 0.68f, 0.0f),
                          glm::vec3(2.8f, 0.12f, 0.80f), bodyBlue);

        // Cow-catcher (angled front plate)
        renderer.drawCube(loco + glm::vec3(dir * 2.25f, -0.18f, 0.0f),
                          glm::vec3(0.48f, 0.24f, 0.86f), cowcatch);

        // Red accent stripe
        renderer.drawCube(loco + glm::vec3(dir * 0.3f, -0.22f, 0.0f),
                          glm::vec3(2.8f, 0.10f, 0.90f), redStripe);

        // Smokestack base
        renderer.drawCube(loco + glm::vec3(dir * 1.4f, 0.68f, 0.0f),
                          glm::vec3(0.28f, 0.36f, 0.28f), stackCol);
        // Stack flared cap
        renderer.drawCube(loco + glm::vec3(dir * 1.4f, 0.90f, 0.0f),
                          glm::vec3(0.42f, 0.10f, 0.42f), stackCol);
        // Steam puff (small white cube above stack – animated bobbing could
        // be added later; for now it gives visual mass to the smokestack)
        renderer.drawCube(loco + glm::vec3(dir * 1.4f, 1.05f, 0.0f),
                          glm::vec3(0.34f, 0.22f, 0.34f), steamCol);
        renderer.drawCube(loco + glm::vec3(dir * 1.55f, 1.22f, 0.0f),
                          glm::vec3(0.22f, 0.16f, 0.22f), steamCol);

        // Front headlamp
        renderer.drawCube(loco + glm::vec3(dir * 2.15f, 0.14f, 0.0f),
                          glm::vec3(0.24f, 0.22f, 0.22f), headlamp);

        // Cab side windows
        renderer.drawCube(loco + glm::vec3(-dir * 0.30f, 0.50f,  0.38f),
                          glm::vec3(1.20f, 0.28f, 0.08f), darkWin);
        renderer.drawCube(loco + glm::vec3(-dir * 0.30f, 0.50f, -0.38f),
                          glm::vec3(1.20f, 0.28f, 0.08f), darkWin);

        // Front window
        renderer.drawCube(loco + glm::vec3(dir * 2.0f, 0.48f, 0.0f),
                          glm::vec3(0.08f, 0.24f, 0.60f), darkWin);

        // Loco wheels — 3 pairs
        float locoWheelX[3] = { dir * 1.6f, dir * 0.2f, -dir * 1.1f };
        for (int w = 0; w < 3; w++) {
            renderer.drawCube(loco + glm::vec3(locoWheelX[w], -0.32f,  0.43f),
                              glm::vec3(0.28f, 0.28f, 0.22f), wheelCol);
            renderer.drawCube(loco + glm::vec3(locoWheelX[w], -0.32f, -0.43f),
                              glm::vec3(0.28f, 0.28f, 0.22f), wheelCol);
        }
        // Connecting rods
        renderer.drawCube(loco + glm::vec3(dir * 0.9f, -0.30f,  0.43f),
                          glm::vec3(1.40f, 0.06f, 0.06f), glm::vec3(0.55f, 0.55f, 0.58f));
        renderer.drawCube(loco + glm::vec3(dir * 0.9f, -0.30f, -0.43f),
                          glm::vec3(1.40f, 0.06f, 0.06f), glm::vec3(0.55f, 0.55f, 0.58f));

        // ════════════════════════════════════════════════════════════════════
        //  Helper lambda (inline): draw one passenger car
        // ════════════════════════════════════════════════════════════════════
        auto drawCar = [&](glm::vec3 centre) {
            // Main body
            renderer.drawCube(centre,
                              glm::vec3(4.7f, 0.52f, 0.86f), carBody);
            // Roof stripe
            renderer.drawCube(centre + glm::vec3(0.0f, 0.30f, 0.0f),
                              glm::vec3(4.7f, 0.10f, 0.84f), bodyBlue);
            // Lower accent stripe
            renderer.drawCube(centre + glm::vec3(0.0f, -0.24f, 0.0f),
                              glm::vec3(4.7f, 0.08f, 0.88f), bodyBlue);
            // Window row – 3 windows per side
            float winX[3] = { -1.35f, 0.0f, 1.35f };
            for (int w = 0; w < 3; w++) {
                renderer.drawCube(centre + glm::vec3(winX[w], 0.10f,  0.37f),
                                  glm::vec3(0.65f, 0.22f, 0.08f), darkWin);
                renderer.drawCube(centre + glm::vec3(winX[w], 0.10f, -0.37f),
                                  glm::vec3(0.65f, 0.22f, 0.08f), darkWin);
            }
            // Wheels – 2 axles
            float wX[2] = { 1.4f, -1.4f };
            for (int w = 0; w < 2; w++) {
                renderer.drawCube(centre + glm::vec3(wX[w], -0.32f,  0.42f),
                                  glm::vec3(0.26f, 0.26f, 0.22f), wheelCol);
                renderer.drawCube(centre + glm::vec3(wX[w], -0.32f, -0.42f),
                                  glm::vec3(0.26f, 0.26f, 0.22f), wheelCol);
            }
        };

        drawCar(c1);
        drawCar(c2);
        drawCar(c3);

        // Rear tail-light on last car
        renderer.drawCube(c3 + glm::vec3(-dir * 2.42f, 0.0f, 0.0f),
                          glm::vec3(0.10f, 0.30f, 0.70f), redStripe);

        return;
    }

    // ── LILYPAD ──────────────────────────────────────────────────────────────
    if (type == OBSTACLE_LILYPAD) {
        glm::vec3 darkCenter(0.25f, 0.69f, 0.37f);
        glm::vec3 lightEdge (0.36f, 0.87f, 0.51f);
        renderer.drawLilypad(position, size, darkCenter, lightEdge);
        return;
    }

    // ── CARS ─────────────────────────────────────────────────────────────────
    const float dir = (speed >= 0.0f) ? 1.0f : -1.0f;
    glm::vec3   pos = position;

    if (vehicleVariant == VEHICLE_SMALL_CAR) {
        glm::vec3 body  (0.28f, 0.82f, 0.42f);
        glm::vec3 roof  (0.18f, 0.58f, 0.28f);
        glm::vec3 win   (0.08f, 0.08f, 0.08f);

        renderer.drawCube(pos + glm::vec3(0.0f,  0.18f, 0.0f),
                          glm::vec3(1.40f, 0.32f, 0.74f), body);
        renderer.drawCube(pos + glm::vec3(0.12f * dir, 0.42f, 0.0f),
                          glm::vec3(0.80f, 0.24f, 0.58f), roof);
        renderer.drawCube(pos + glm::vec3(0.34f * dir, 0.48f, 0.0f),
                          glm::vec3(0.22f, 0.16f, 0.44f), win);
        drawWheels(renderer, pos, -0.48f, 0.48f, 0.34f, 0.15f);
    }
    else if (vehicleVariant == VEHICLE_BIG_CAR) {
        glm::vec3 body  (0.95f, 0.38f, 0.08f);
        glm::vec3 roof  (0.96f, 0.96f, 0.96f);
        glm::vec3 win   (0.08f, 0.08f, 0.08f);

        renderer.drawCube(pos + glm::vec3(0.0f,  0.18f, 0.0f),
                          glm::vec3(1.90f, 0.36f, 0.86f), body);
        renderer.drawCube(pos + glm::vec3(0.0f,  0.46f, 0.0f),
                          glm::vec3(1.90f, 0.25f, 0.80f), roof);
        renderer.drawCube(pos + glm::vec3(0.68f * dir, 0.50f, 0.0f),
                          glm::vec3(0.28f, 0.20f, 0.52f), win);
        renderer.drawCube(pos + glm::vec3(-0.60f * dir, 0.50f, 0.0f),
                          glm::vec3(0.24f, 0.20f, 0.52f), win);
        drawWheels(renderer, pos, -0.68f, 0.68f, 0.38f, 0.17f);
    }
    else {
        glm::vec3 trailer(0.88f, 0.88f, 0.92f);
        glm::vec3 cab    (0.18f, 0.52f, 0.92f);
        glm::vec3 win    (0.08f, 0.08f, 0.08f);

        renderer.drawCube(pos + glm::vec3(-0.55f * dir, 0.42f, 0.0f),
                          glm::vec3(2.10f, 0.72f, 0.84f), trailer);
        renderer.drawCube(pos + glm::vec3(1.10f * dir, 0.34f, 0.0f),
                          glm::vec3(0.88f, 0.56f, 0.84f), cab);
        renderer.drawCube(pos + glm::vec3(1.05f * dir, 0.66f, 0.0f),
                          glm::vec3(0.62f, 0.20f, 0.60f), cab);
        renderer.drawCube(pos + glm::vec3(1.36f * dir, 0.52f, 0.0f),
                          glm::vec3(0.20f, 0.24f, 0.42f), win);

        float w = 0.20f;
        glm::vec3 wc(0.08f, 0.08f, 0.08f);
        renderer.drawCube(pos + glm::vec3( 0.90f * dir, -0.06f,  0.38f), glm::vec3(w), wc);
        renderer.drawCube(pos + glm::vec3( 0.90f * dir, -0.06f, -0.38f), glm::vec3(w), wc);
        renderer.drawCube(pos + glm::vec3(-0.20f * dir, -0.06f,  0.38f), glm::vec3(w), wc);
        renderer.drawCube(pos + glm::vec3(-0.20f * dir, -0.06f, -0.38f), glm::vec3(w), wc);
        renderer.drawCube(pos + glm::vec3(-0.90f * dir, -0.06f,  0.38f), glm::vec3(w), wc);
        renderer.drawCube(pos + glm::vec3(-0.90f * dir, -0.06f, -0.38f), glm::vec3(w), wc);
    }
}