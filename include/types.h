#ifndef TYPES_H
#define TYPES_H

#include <glm/glm.hpp>

constexpr float CELL_SIZE = 1.0f;

enum GameState {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
};

enum LaneType {
    LANE_GRASS,
    LANE_ROAD,
    LANE_RIVER,
    LANE_RAIL
};

enum ObstacleType {
    OBSTACLE_CAR,
    OBSTACLE_TRUCK,
    OBSTACLE_TRAIN,
    OBSTACLE_LOG,
    OBSTACLE_LILYPAD
};

struct GameData {
    int score;
    int coins;
    int health;
    int level;
    glm::vec3 playerPos;
    bool isGameOver;
};

#endif