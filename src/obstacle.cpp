#include "../include/obstacle.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────
Obstacle::Obstacle(glm::vec3 startPos, float spd, ObstacleType t, VehicleVariant variant)
    : position(startPos), speed(spd), type(t), vehicleVariant(variant),
      isActive(true), sinkOffset(0.0f), isSinking(false)
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
    if (!isActive) return;

    position.x += speed * deltaTime;

    // Wrap / deactivate
    if (type == OBSTACLE_TRAIN) {
        if ((speed > 0 && position.x > 60.0f) || (speed < 0 && position.x < -60.0f))
            isActive = false;
    } else {
        if (speed > 0 && position.x >  15.0f) position.x = -15.0f;
        if (speed < 0 && position.x < -15.0f) position.x =  15.0f;
    }

    // ── Log sinking effect ──────────────────────────────────────────────────
    // isSinking is set to true by Game::checkCollisions each frame the chicken
    // stands on this log.  We reset it here after lerping so it auto-recovers
    // when the chicken jumps off.
    if (type == OBSTACLE_LOG) {
        float target = isSinking ? -Config::LOG_SINK_AMOUNT : 0.0f;
        sinkOffset += (target - sinkOffset) * Config::LOG_SINK_SPEED * deltaTime;
        isSinking    = false;   // cleared every frame; game re-sets it if needed
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
        renderPos.y += sinkOffset;          // sinking effect
        renderer.drawTexturedCube(renderPos, size, "log");
        return;
    }

    // ── TRAIN ────────────────────────────────────────────────────────────────
    if (type == OBSTACLE_TRAIN) {
        renderer.drawTexturedCube(position, size, "train");
        return;
    }

    // ── LILYPAD ──────────────────────────────────────────────────────────────
    if (type == OBSTACLE_LILYPAD) {
        // Dark forest green for the center, bright yellow-green for the edges
        glm::vec3 darkCenter(0.25f, 0.69f, 0.37f);
        glm::vec3 lightEdge(0.36f, 0.87f, 0.51f);  

        renderer.drawLilypad(position, size, darkCenter, lightEdge);
        return;
    }

    // ── CARS ─────────────────────────────────────────────────────────────────
    // dir = +1 when moving right, -1 when moving left.
    // The "front" of each vehicle faces in the direction of travel.
    const float dir = (speed >= 0.0f) ? 1.0f : -1.0f;
    glm::vec3   pos = position;

    // ── SMALL CAR  (compact, rounded-style voxel) ───────────────────────────
    if (vehicleVariant == VEHICLE_SMALL_CAR) {

        // ── Palette  (feel free to swap colors here) ─────────────────────
        glm::vec3 body  (0.28f, 0.82f, 0.42f);   // green
        glm::vec3 roof  (0.18f, 0.58f, 0.28f);   // darker green
        glm::vec3 win   (0.08f, 0.08f, 0.08f);   // near-black window

        // Body slab
        renderer.drawCube(pos + glm::vec3(0.0f,  0.18f, 0.0f),
                          glm::vec3(1.40f, 0.32f, 0.74f), body);
        // Roof cab
        renderer.drawCube(pos + glm::vec3(0.12f * dir, 0.42f, 0.0f),
                          glm::vec3(0.80f, 0.24f, 0.58f), roof);
        // Windshield
        renderer.drawCube(pos + glm::vec3(0.34f * dir, 0.48f, 0.0f),
                          glm::vec3(0.22f, 0.16f, 0.44f), win);
        // Wheels
        drawWheels(renderer, pos, -0.48f, 0.48f, 0.34f, 0.15f);
    }

    // ── BIG CAR  (orange SUV / people-carrier like the reference image) ─────
    else if (vehicleVariant == VEHICLE_BIG_CAR) {

        glm::vec3 body  (0.95f, 0.38f, 0.08f);   // orange
        glm::vec3 roof  (0.96f, 0.96f, 0.96f);   // white roof section
        glm::vec3 win   (0.08f, 0.08f, 0.08f);

        // Lower body
        renderer.drawCube(pos + glm::vec3(0.0f,  0.18f, 0.0f),
                          glm::vec3(1.90f, 0.36f, 0.86f), body);
        // Upper/roof cab (white, flush with body width to look boxy)
        renderer.drawCube(pos + glm::vec3(0.0f,  0.46f, 0.0f),
                          glm::vec3(1.90f, 0.28f, 0.80f), roof);
        // Front window
        renderer.drawCube(pos + glm::vec3(0.68f * dir, 0.50f, 0.0f),
                          glm::vec3(0.28f, 0.20f, 0.52f), win);
        // Rear window
        renderer.drawCube(pos + glm::vec3(-0.60f * dir, 0.50f, 0.0f),
                          glm::vec3(0.24f, 0.20f, 0.52f), win);
        // Wheels
        drawWheels(renderer, pos, -0.68f, 0.68f, 0.38f, 0.17f);
    }

    

    // ── TRUCK  (blue cab + white trailer, exactly like the reference image) ──
    else {

        glm::vec3 trailer(0.88f, 0.88f, 0.92f);  // light grey-white
        glm::vec3 cab    (0.18f, 0.52f, 0.92f);  // blue
        glm::vec3 win    (0.08f, 0.08f, 0.08f);

        // ── Trailer (behind the cab) ─────────────────────────────────────
        renderer.drawCube(pos + glm::vec3(-0.55f * dir, 0.42f, 0.0f),
                          glm::vec3(2.10f, 0.72f, 0.84f), trailer);

        // ── Cab ──────────────────────────────────────────────────────────
        renderer.drawCube(pos + glm::vec3(1.10f * dir, 0.34f, 0.0f),
                          glm::vec3(0.88f, 0.56f, 0.84f), cab);
        // Cab roof
        renderer.drawCube(pos + glm::vec3(1.05f * dir, 0.66f, 0.0f),
                          glm::vec3(0.62f, 0.20f, 0.60f), cab);
        // Windshield
        renderer.drawCube(pos + glm::vec3(1.36f * dir, 0.52f, 0.0f),
                          glm::vec3(0.20f, 0.24f, 0.42f), win);

        // ── Wheels (cab pair + trailer pair) ─────────────────────────────
        float w = 0.20f;
        glm::vec3 wc(0.08f, 0.08f, 0.08f);
        // Cab wheels
        renderer.drawCube(pos + glm::vec3( 0.90f * dir, -0.06f,  0.38f), glm::vec3(w), wc);
        renderer.drawCube(pos + glm::vec3( 0.90f * dir, -0.06f, -0.38f), glm::vec3(w), wc);
        // Trailer wheels (two axles)
        renderer.drawCube(pos + glm::vec3(-0.20f * dir, -0.06f,  0.38f), glm::vec3(w), wc);
        renderer.drawCube(pos + glm::vec3(-0.20f * dir, -0.06f, -0.38f), glm::vec3(w), wc);
        renderer.drawCube(pos + glm::vec3(-0.90f * dir, -0.06f,  0.38f), glm::vec3(w), wc);
        renderer.drawCube(pos + glm::vec3(-0.90f * dir, -0.06f, -0.38f), glm::vec3(w), wc);
    }
}