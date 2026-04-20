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
    struct SignalPost {
        glm::vec3 position;   // world-space centre (base of pole)
    };
    std::vector<SignalPost> signalPosts; 
    int safePathColumn; // 🔥 NEW
    // int blockWidth;
    // int lane_idx;

public:
    Lane(float z, LaneType t, int safePath); // 🔥 UPDATED
    void update(float deltaTime);
    void render(Renderer& renderer, float sunAngle = 0.0f, int frameTime = 0);  // OPTIMIZATION: frameTime for caching

    const std::vector<Obstacle>& getObstacles() const { return obstacles; }
    std::vector<Obstacle>& getObstacles() { return obstacles; }
    LaneType getType() const { return type; }
    float getZPosition() const { return zPosition; }
};

#endif