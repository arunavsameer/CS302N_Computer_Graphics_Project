#include "../include/obstacle.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────
Obstacle::Obstacle(glm::vec3 startPos, float spd, ObstacleType t, VehicleVariant variant)
    : position(startPos), speed(spd), type(t), vehicleVariant(variant),
      isActive(true), sinkOffset(0.0f), isSinking(false),
      startPosition(startPos), respawnTimer(0.0f), fastStream(false)
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

    // ── TRAIN ─────────────────────────────────────────────────────────────────
    if (type == OBSTACLE_TRAIN) {
        if (isActive) {
            position.x += speed * deltaTime;
            if ((speed > 0 && position.x > 65.0f) || (speed < 0 && position.x < -65.0f)) {
                isActive     = false;
                respawnTimer = respawnDelay;
            }
        } else {
            respawnTimer -= deltaTime;
            if (respawnTimer <= 0.0f) {
                position = startPosition;
                isActive = true;
            }
        }
        return;
    }

    if (!isActive) return;

    // ── LOG ───────────────────────────────────────────────────────────────────
    if (type == OBSTACLE_LOG) {
        // Fast-stream: exponentially accelerate toward speed cap
        if (fastStream) {
            float sign     = (speed >= 0.0f) ? 1.0f : -1.0f;
            float absSpeed = std::abs(speed);
            absSpeed += absSpeed * Config::LOG_STREAM_ACCEL * deltaTime;
            absSpeed  = std::min(absSpeed, Config::LOG_STREAM_MAX_SPEED);
            speed     = sign * absSpeed;

            position.x += speed * deltaTime;

            // De-activate when far off screen (no wrap in fast-stream)
            if (std::abs(position.x) > Config::LOG_STREAM_EXIT_X) {
                isActive    = false;
                fastStream  = false;
                // Reset log to opposite side so it re-appears naturally
                position.x  = (speed > 0) ? -15.0f : 15.0f;
                // Restore original base speed
                speed = startPosition.x; // reuse startPosition as speed store?
                // Actually restore properly:
                speed = (speed > 0 ? 1.0f : -1.0f)
                        * (Config::LOG_SPEED_MIN +
                           static_cast<float>(rand()) /
                           static_cast<float>(RAND_MAX) *
                           (Config::LOG_SPEED_MAX - Config::LOG_SPEED_MIN));
                isActive = true;
            }
        } else {
            // Normal log movement with standard wrap
            position.x += speed * deltaTime;
            if (speed > 0 && position.x >  15.0f) position.x = -15.0f;
            if (speed < 0 && position.x < -15.0f) position.x =  15.0f;
        }

        // Sink offset lerp
        float target  = isSinking ? -Config::LOG_SINK_AMOUNT : 0.0f;
        sinkOffset   += (target - sinkOffset) * Config::LOG_SINK_SPEED * deltaTime;
        isSinking     = false;
        return;
    }

    // ── CARS (and lilypad, no-op for lilypad) ────────────────────────────────
    position.x += speed * deltaTime;
    if (speed > 0 && position.x >  15.0f) position.x = -15.0f;
    if (speed < 0 && position.x < -15.0f) position.x =  15.0f;
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

    // ── TRAIN ─────────────────────────────────────────────────────────────────
    if (type == OBSTACLE_TRAIN) {
        const float dir = (speed >= 0.0f) ? 1.0f : -1.0f;
        const glm::vec3 p = position;

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

        glm::vec3 loco(p.x + dir * 7.75f, p.y, p.z);
        glm::vec3 c1  (p.x + dir * 2.85f, p.y, p.z);
        glm::vec3 c2  (p.x - dir * 2.15f, p.y, p.z);
        glm::vec3 c3  (p.x - dir * 7.15f, p.y, p.z);

        renderer.drawCube(loco, {4.5f,0.58f,0.88f}, bodyBlue);
        renderer.drawCube(loco+glm::vec3(-dir*0.65f, 0.44f,0), {3.0f,0.36f,0.82f}, roofWhite);
        renderer.drawCube(loco+glm::vec3(-dir*0.65f, 0.68f,0), {2.8f,0.12f,0.80f}, bodyBlue);
        renderer.drawCube(loco+glm::vec3(dir*2.25f,-0.18f,0),  {0.48f,0.24f,0.86f}, cowcatch);
        renderer.drawCube(loco+glm::vec3(dir*0.3f,-0.22f,0),   {2.8f,0.10f,0.90f}, redStripe);
        renderer.drawCube(loco+glm::vec3(dir*1.4f, 0.68f,0),   {0.28f,0.36f,0.28f}, stackCol);
        renderer.drawCube(loco+glm::vec3(dir*1.4f, 0.90f,0),   {0.42f,0.10f,0.42f}, stackCol);
        renderer.drawCube(loco+glm::vec3(dir*1.4f, 1.05f,0),   {0.34f,0.22f,0.34f}, steamCol);
        renderer.drawCube(loco+glm::vec3(dir*1.55f,1.22f,0),   {0.22f,0.16f,0.22f}, steamCol);
        // ── Two emissive headlamp lenses (replaces the old single headlamp) ──
        renderer.drawCubeEmissive(loco+glm::vec3(dir*2.15f, 0.14f,  0.18f), {0.18f,0.18f,0.14f}, headlamp);
        renderer.drawCubeEmissive(loco+glm::vec3(dir*2.15f, 0.14f, -0.18f), {0.18f,0.18f,0.14f}, headlamp);
        if (renderer.isNightMode())
            renderer.drawHeadlightBeam(loco + glm::vec3(dir*2.15f, 0.14f, 0.0f),
                                       dir, 1.40f, 0.55f, 8.0f);
        renderer.drawCube(loco+glm::vec3(-dir*0.30f,0.50f, 0.38f), {1.20f,0.28f,0.08f}, darkWin);
        renderer.drawCube(loco+glm::vec3(-dir*0.30f,0.50f,-0.38f), {1.20f,0.28f,0.08f}, darkWin);
        renderer.drawCube(loco+glm::vec3(dir*2.0f,  0.48f,0),      {0.08f,0.24f,0.60f}, darkWin);

        float locoWheelX[3] = { dir*1.6f, dir*0.2f, -dir*1.1f };
        for (int w = 0; w < 3; w++) {
            renderer.drawCube(loco+glm::vec3(locoWheelX[w],-0.32f, 0.43f), {0.28f,0.28f,0.22f}, wheelCol);
            renderer.drawCube(loco+glm::vec3(locoWheelX[w],-0.32f,-0.43f), {0.28f,0.28f,0.22f}, wheelCol);
        }
        renderer.drawCube(loco+glm::vec3(dir*0.9f,-0.30f, 0.43f), {1.40f,0.06f,0.06f}, {0.55f,0.55f,0.58f});
        renderer.drawCube(loco+glm::vec3(dir*0.9f,-0.30f,-0.43f), {1.40f,0.06f,0.06f}, {0.55f,0.55f,0.58f});

        auto drawCar = [&](glm::vec3 centre) {
            renderer.drawCube(centre,  {4.7f,0.52f,0.86f}, carBody);
            renderer.drawCube(centre+glm::vec3(0,0.30f,0), {4.7f,0.10f,0.84f}, bodyBlue);
            renderer.drawCube(centre+glm::vec3(0,-0.24f,0),{4.7f,0.08f,0.88f}, bodyBlue);
            float winX[3]={-1.35f,0.0f,1.35f};
            for (int w=0;w<3;w++){
                renderer.drawCube(centre+glm::vec3(winX[w],0.10f, 0.37f),{0.65f,0.22f,0.08f},darkWin);
                renderer.drawCube(centre+glm::vec3(winX[w],0.10f,-0.37f),{0.65f,0.22f,0.08f},darkWin);
            }
            float wX[2]={1.4f,-1.4f};
            for (int w=0;w<2;w++){
                renderer.drawCube(centre+glm::vec3(wX[w],-0.32f, 0.42f),{0.26f,0.26f,0.22f},wheelCol);
                renderer.drawCube(centre+glm::vec3(wX[w],-0.32f,-0.42f),{0.26f,0.26f,0.22f},wheelCol);
            }
        };
        drawCar(c1); drawCar(c2); drawCar(c3);
        renderer.drawCube(c3+glm::vec3(-dir*2.42f,0,0), {0.10f,0.30f,0.70f}, redStripe);
        return;
    }

    // ── LILYPAD ───────────────────────────────────────────────────────────────
    if (type == OBSTACLE_LILYPAD) {
        renderer.drawLilypad(position, size,
                             {0.25f,0.69f,0.37f}, {0.36f,0.87f,0.51f});
        return;
    }

    // ── CARS ──────────────────────────────────────────────────────────────────
    const float dir = (speed >= 0.0f) ? 1.0f : -1.0f;
    glm::vec3   pos = position;

    if (vehicleVariant == VEHICLE_SMALL_CAR) {
        glm::vec3 body(0.28f,0.82f,0.42f), roof(0.18f,0.58f,0.28f), win(0.08f,0.08f,0.08f);
        renderer.drawCube(pos+glm::vec3(0,0.18f,0),          {1.40f,0.32f,0.74f},body);
        renderer.drawCube(pos+glm::vec3(0.12f*dir,0.42f,0),  {0.80f,0.24f,0.58f},roof);
        renderer.drawCube(pos+glm::vec3(0.34f*dir,0.48f,0),  {0.22f,0.16f,0.44f},win);
        drawWheels(renderer,pos,-0.48f,0.48f,0.34f,0.15f);
        // ── Headlights ────────────────────────────────────────────────────
        glm::vec3 hlCol(1.00f, 0.97f, 0.75f);
        glm::vec3 hlSize(0.07f, 0.10f, 0.10f);
        float frontX = 0.70f * dir;
        renderer.drawCubeEmissive(pos + glm::vec3(frontX,  0.18f,  0.24f), hlSize, hlCol);
        renderer.drawCubeEmissive(pos + glm::vec3(frontX,  0.18f, -0.24f), hlSize, hlCol);
        if (renderer.isNightMode())
            renderer.drawHeadlightBeam(pos + glm::vec3(frontX, 0.18f, 0.0f),
                                       dir, 0.90f, 0.35f, 5.0f);
    }
    else if (vehicleVariant == VEHICLE_BIG_CAR) {
        glm::vec3 body(0.95f,0.38f,0.08f), roof(0.96f,0.96f,0.96f), win(0.08f,0.08f,0.08f);
        renderer.drawCube(pos+glm::vec3(0,0.18f,0),          {1.90f,0.36f,0.86f},body);
        renderer.drawCube(pos+glm::vec3(0,0.46f,0),          {1.90f,0.25f,0.80f},roof);
        renderer.drawCube(pos+glm::vec3(0.68f*dir,0.50f,0),  {0.28f,0.20f,0.52f},win);
        renderer.drawCube(pos+glm::vec3(-0.60f*dir,0.50f,0), {0.24f,0.20f,0.52f},win);
        drawWheels(renderer,pos,-0.68f,0.68f,0.38f,0.17f);
        // ── Headlights ────────────────────────────────────────────────────
        glm::vec3 hlCol(1.00f, 0.97f, 0.75f);
        glm::vec3 hlSize(0.07f, 0.12f, 0.12f);
        float frontX = 0.95f * dir;
        renderer.drawCubeEmissive(pos + glm::vec3(frontX,  0.18f,  0.30f), hlSize, hlCol);
        renderer.drawCubeEmissive(pos + glm::vec3(frontX,  0.18f, -0.30f), hlSize, hlCol);
        if (renderer.isNightMode())
            renderer.drawHeadlightBeam(pos + glm::vec3(frontX, 0.18f, 0.0f),
                                       dir, 1.10f, 0.40f, 5.5f);
    }
    else {  // TRUCK
        glm::vec3 trailer(0.88f,0.88f,0.92f), cab(0.18f,0.52f,0.92f), win(0.08f,0.08f,0.08f);
        renderer.drawCube(pos+glm::vec3(-0.55f*dir,0.42f,0), {2.10f,0.72f,0.84f},trailer);
        renderer.drawCube(pos+glm::vec3( 1.10f*dir,0.34f,0), {0.88f,0.56f,0.84f},cab);
        renderer.drawCube(pos+glm::vec3( 1.05f*dir,0.66f,0), {0.62f,0.20f,0.60f},cab);
        renderer.drawCube(pos+glm::vec3( 1.36f*dir,0.52f,0), {0.20f,0.24f,0.42f},win);
        float w=0.20f; glm::vec3 wc(0.08f,0.08f,0.08f);
        renderer.drawCube(pos+glm::vec3( 0.90f*dir,-0.06f, 0.38f),{w,w,w},wc);
        renderer.drawCube(pos+glm::vec3( 0.90f*dir,-0.06f,-0.38f),{w,w,w},wc);
        renderer.drawCube(pos+glm::vec3(-0.20f*dir,-0.06f, 0.38f),{w,w,w},wc);
        renderer.drawCube(pos+glm::vec3(-0.20f*dir,-0.06f,-0.38f),{w,w,w},wc);
        renderer.drawCube(pos+glm::vec3(-0.90f*dir,-0.06f, 0.38f),{w,w,w},wc);
        renderer.drawCube(pos+glm::vec3(-0.90f*dir,-0.06f,-0.38f),{w,w,w},wc);
        // ── Headlights (on cab front face) ────────────────────────────────
        glm::vec3 hlCol(1.00f, 0.97f, 0.75f);
        glm::vec3 hlSize(0.07f, 0.11f, 0.11f);
        float frontX = 1.54f * dir;   // cab front: 1.10 + 0.44 half-width
        renderer.drawCubeEmissive(pos + glm::vec3(frontX,  0.28f,  0.26f), hlSize, hlCol);
        renderer.drawCubeEmissive(pos + glm::vec3(frontX,  0.28f, -0.26f), hlSize, hlCol);
        if (renderer.isNightMode())
            renderer.drawHeadlightBeam(pos + glm::vec3(frontX, 0.28f, 0.0f),
                                       dir, 1.20f, 0.45f, 6.0f);
    }
}