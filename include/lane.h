#ifndef LANE_H
#define LANE_H

#include <vector>
#include "types.h"
#include "obstacle.h"
#include "renderer.h"

class Lane {
private:
    float zPosition;
    LaneType type;
    std::vector<Obstacle> obstacles;

public:
    Lane(float z, LaneType t);
    void update(float deltaTime);
    void render(Renderer& renderer);
};

#endif