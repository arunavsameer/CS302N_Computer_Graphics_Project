#include "../include/obstacle.h"

static void drawWheel(Renderer& renderer, const glm::vec3& pos, float radiusX, float radiusY, float radiusZ) {
    renderer.drawCube(pos, glm::vec3(radiusX, radiusY, radiusZ), glm::vec3(0.08f, 0.08f, 0.08f));
}

Obstacle::Obstacle(glm::vec3 startPos, float spd, ObstacleType t, VehicleVariant variant)
    : position(startPos), speed(spd), type(t), vehicleVariant(variant), isActive(true) {
    
    if (type == OBSTACLE_CAR) {
        if (vehicleVariant == VEHICLE_SMALL_CAR) {
            size = glm::vec3(1.45f, 0.62f, 0.78f);
        } else if (vehicleVariant == VEHICLE_BIG_CAR) {
            size = glm::vec3(1.95f, 0.72f, 0.88f);
        } else {
            size = glm::vec3(3.05f, 0.92f, 1.00f);
        }
    } else if (type == OBSTACLE_TRAIN) {
        size = glm::vec3(Config::TRAIN_LENGTH * Config::CELL_SIZE, Config::CELL_SIZE * 1.0f, Config::CELL_SIZE * 0.9f);
    } else if (type == OBSTACLE_LOG) {
        size = glm::vec3(Config::CELL_SIZE * 3.0f, Config::CELL_SIZE * 0.4f, Config::CELL_SIZE * 0.8f);
    }
}

void Obstacle::update(float deltaTime) {
    if (!isActive) return;

    position.x += speed * deltaTime;
    
    if (type == OBSTACLE_TRAIN) {
        if ((speed > 0 && position.x > 60.0f) || (speed < 0 && position.x < -60.0f)) {
            isActive = false; 
        }
    } else {
        if (speed > 0 && position.x > 15.0f) position.x = -15.0f;
        if (speed < 0 && position.x < -15.0f) position.x = 15.0f;
    }
}

void Obstacle::render(Renderer& renderer) {
    if (!isActive) return;

    if (type == OBSTACLE_CAR) {
        float dir = (speed >= 0.0f) ? 1.0f : -1.0f;
        glm::vec3 pos = position;

        // ================= SMALL CAR =================
        if (vehicleVariant == VEHICLE_SMALL_CAR) {

            glm::vec3 bodyColor(0.30f, 0.85f, 0.45f);
            glm::vec3 roofColor(0.18f, 0.60f, 0.30f);
            glm::vec3 windowColor(0.05f, 0.05f, 0.05f);

            // BODY
            renderer.drawCube(pos + glm::vec3(0, 0.18f, 0),
                              glm::vec3(1.4f, 0.35f, 0.75f),
                              bodyColor);

            // ROOF
            renderer.drawCube(pos + glm::vec3(0.15f * dir, 0.45f, 0),
                              glm::vec3(0.8f, 0.25f, 0.6f),
                              roofColor);

            // WINDOW
            renderer.drawCube(pos + glm::vec3(0.35f * dir, 0.52f, 0),
                              glm::vec3(0.3f, 0.15f, 0.5f),
                              windowColor);

            // WHEELS
            float w = 0.15f;
            renderer.drawCube(pos + glm::vec3(-0.5f, -0.05f, 0.35f), glm::vec3(w), glm::vec3(0.05f));
            renderer.drawCube(pos + glm::vec3(0.5f, -0.05f, 0.35f), glm::vec3(w), glm::vec3(0.05f));
            renderer.drawCube(pos + glm::vec3(-0.5f, -0.05f, -0.35f), glm::vec3(w), glm::vec3(0.05f));
            renderer.drawCube(pos + glm::vec3(0.5f, -0.05f, -0.35f), glm::vec3(w), glm::vec3(0.05f));
        }

        // ================= BIG CAR (TAXI STYLE) =================
        else if (vehicleVariant == VEHICLE_BIG_CAR) {

            glm::vec3 bodyColor(1.0f, 0.85f, 0.2f);
            glm::vec3 roofColor(0.95f, 0.75f, 0.15f);
            glm::vec3 windowColor(0.1f, 0.1f, 0.1f);

            // BODY
            renderer.drawCube(pos + glm::vec3(0, 0.2f, 0),
                              glm::vec3(1.9f, 0.4f, 0.85f),
                              bodyColor);

            // ROOF
            renderer.drawCube(pos + glm::vec3(0.1f * dir, 0.48f, 0),
                              glm::vec3(1.0f, 0.25f, 0.7f),
                              roofColor);

            // TAXI LIGHT
            renderer.drawCube(pos + glm::vec3(0.1f * dir, 0.65f, 0),
                              glm::vec3(0.3f, 0.1f, 0.3f),
                              glm::vec3(1.0f, 0.9f, 0.3f));

            // WINDOW
            renderer.drawCube(pos + glm::vec3(0.4f * dir, 0.55f, 0),
                              glm::vec3(0.35f, 0.18f, 0.55f),
                              windowColor);

            // WHEELS
            float w = 0.17f;
            renderer.drawCube(pos + glm::vec3(-0.7f, -0.05f, 0.38f), glm::vec3(w), glm::vec3(0.05f));
            renderer.drawCube(pos + glm::vec3(0.7f, -0.05f, 0.38f), glm::vec3(w), glm::vec3(0.05f));
            renderer.drawCube(pos + glm::vec3(-0.7f, -0.05f, -0.38f), glm::vec3(w), glm::vec3(0.05f));
            renderer.drawCube(pos + glm::vec3(0.7f, -0.05f, -0.38f), glm::vec3(w), glm::vec3(0.05f));
        }

        // ================= TRUCK =================
        // ================= TRUCK (FINAL FIXED) =================
else {

    glm::vec3 trailerColor(0.96f, 0.96f, 0.98f);
    glm::vec3 cabColor(0.95f, 0.2f, 0.2f);
    glm::vec3 windowColor(0.1f, 0.1f, 0.1f);

    float dir = (speed >= 0.0f) ? 1.0f : -1.0f;

    // ===== TRAILER (slightly narrower to avoid overlap)
    renderer.drawCube(pos + glm::vec3(-0.45f * dir, 0.38f, 0),
                      glm::vec3(2.0f, 0.75f, 0.85f),
                      trailerColor);

    // ===== CAB (shifted + smaller footprint)
    renderer.drawCube(pos + glm::vec3(1.2f * dir, 0.32f, 0),
                      glm::vec3(0.85f, 0.6f, 0.85f),
                      cabColor);

    // ===== CAB ROOF (SEPARATED → no flicker)
    renderer.drawCube(pos + glm::vec3(1.2f * dir, 0.65f, 0),
                      glm::vec3(0.6f, 0.2f, 0.6f),
                      cabColor);

    // ===== WINDOW (slightly inside → no overlap)
    renderer.drawCube(pos + glm::vec3(1.4f * dir, 0.55f, 0),
                      glm::vec3(0.25f, 0.2f, 0.45f),
                      windowColor);

    // ===== WHEELS (clean spacing)
    float w = 0.2f;

    float offsets[] = {-0.9f, -0.2f, 0.6f, 1.3f};
    for (float off : offsets) {
        renderer.drawCube(pos + glm::vec3(off, -0.05f, 0.38f),
                          glm::vec3(w), glm::vec3(0.05f));

        renderer.drawCube(pos + glm::vec3(off, -0.05f, -0.38f),
                          glm::vec3(w), glm::vec3(0.05f));
    }
}

        return;
    }

    // ===== KEEP ORIGINAL FOR TRAIN/LOG =====
    std::string tex;
    if (type == OBSTACLE_TRAIN) tex = "train";
    else if (type == OBSTACLE_LOG) tex = "log";
    else tex = "road";

    renderer.drawTexturedCube(position, size, tex);
}