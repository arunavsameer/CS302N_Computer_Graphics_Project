#ifndef LANE_H
#define LANE_H

#include <vector>
#include "types.h"
#include "obstacle.h"
#include "renderer.h"
#include "coin.h"

class Lane {
public:
    float zPosition;
    LaneType type;
    std::vector<Obstacle> obstacles;

    std::vector<Coin> coins;

    struct Decoration {
        glm::vec3 position;
        int type;
        float scale;
        glm::vec3 color;
    };

    std::vector<Decoration> decorations;

    int safePathColumn; // 🔥 NEW

public:
    Lane(float z, LaneType t, int safePath); // 🔥 UPDATED

    void update(float deltaTime);
    void render(Renderer& renderer);

    const std::vector<Obstacle>& getObstacles() const { return obstacles; }
    std::vector<Obstacle>& getObstacles() { return obstacles; }
    LaneType getType() const { return type; }
    float getZPosition() const { return zPosition; }
};

#endif