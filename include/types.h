#ifndef TYPES_H
#define TYPES_H

// --- Game Enums ---
enum GameState { GAME_STATE_PLAYING, GAME_STATE_GAME_OVER };
enum LaneType { LANE_GRASS, LANE_ROAD, LANE_RAIL, LANE_RIVER };
enum ObstacleType { OBSTACLE_CAR, OBSTACLE_TRAIN, OBSTACLE_LOG };

// --- Hyperparameters & Config ---
namespace Config {
    constexpr float CELL_SIZE = 1.0f; 
    
    // --- Safe Zone ---
    constexpr int INITIAL_SAFE_ZONE_LENGTH = 15; // Chicken starts deep in safe grass

    // --- Lane Generation Randomness ---
    constexpr int MIN_GRASS_WIDTH = 2;
    constexpr int MAX_GRASS_WIDTH = 4;
    constexpr int MIN_ROAD_WIDTH = 1;
    constexpr int MAX_ROAD_WIDTH = 3;
    constexpr int MIN_RAIL_WIDTH = 1;
    constexpr int MAX_RAIL_WIDTH = 2;
    constexpr int MIN_RIVER_WIDTH = 1;
    constexpr int MAX_RIVER_WIDTH = 3;

    // --- Obstacle Config ---
    constexpr float CAR_SPEED_MIN = 3.0f;
    constexpr float CAR_SPEED_MAX = 7.0f;
    
    constexpr float TRAIN_SPEED = 25.0f; // Very fast
    constexpr float TRAIN_LENGTH = 15.0f; // Very long model/hitbox
    
    constexpr float LOG_SPEED_MIN = 2.0f;
    constexpr float LOG_SPEED_MAX = 4.0f;
}

#endif