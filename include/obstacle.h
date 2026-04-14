#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <glm/glm.hpp>
#include "types.h"
#include "renderer.h"

class Obstacle {
private:
    glm::vec3 position;
    glm::vec3 size;
    float speed;
    ObstacleType type;
    VehicleVariant vehicleVariant;
    bool isActive;
    float sinkOffset = 0.0f;
    bool  isSinking  = false;

    // ── Train respawn ────────────────────────────────────────────────────────
    glm::vec3 startPosition;
    float     respawnTimer;
    float     respawnDelay;

    // ── Fast-stream (log swept into boundary foam) ───────────────────────────
    // Once activated by the game's collision logic, the log exponentially
    // accelerates and does NOT wrap around — it exits the screen entirely.
    bool  fastStream;

public:
    Obstacle(glm::vec3 startPos, float spd, ObstacleType t,
             VehicleVariant variant = VEHICLE_SMALL_CAR);

    void update(float deltaTime);
    void render(Renderer& renderer);

    glm::vec3    getPosition()    const { return position; }
    glm::vec3    getSize()        const { return size; }
    ObstacleType getType()        const { return type; }
    float        getSpeed()       const { return speed; }
    bool         getIsActive()    const { return isActive; }
    bool         getFastStream()  const { return fastStream; }

    void setSinking(bool s)      { isSinking   = s; }
    void setFastStream(bool s)   { fastStream  = s; }
};

#endif