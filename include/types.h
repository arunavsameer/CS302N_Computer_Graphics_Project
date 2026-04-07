#ifndef TYPES_H
#define TYPES_H

// --- Game Enums ---
enum GameState { GAME_STATE_START_SCREEN, GAME_STATE_PLAYING, GAME_STATE_GAME_OVER };
enum LaneType { LANE_GRASS, LANE_ROAD, LANE_RAIL, LANE_RIVER };
enum ObstacleType { OBSTACLE_CAR, OBSTACLE_TRAIN, OBSTACLE_LOG };

struct UIConfig {
    static constexpr int MAX_EGG_CLICKS = 3;
    static constexpr float DEAD_ZOOM_RADIUS = 4.0f;
    static constexpr float DEAD_ZOOM_SPEED = 3.0f; // Slower, dramatic zoom
};
enum VehicleVariant { VEHICLE_SMALL_CAR, VEHICLE_BIG_CAR, VEHICLE_TRUCK };

// --- Hyperparameters & Config ---
namespace Config {
    constexpr float CELL_SIZE = 1.0f; 

    // --- Safe Zone ---
    constexpr int INITIAL_SAFE_ZONE_LENGTH = 15; // Chicken starts deep in safe grass

    constexpr float HITBOX_PADDING = 0.8f;

    // --- Lane Generation Randomness ---
    constexpr int MIN_GRASS_WIDTH = 2;
    constexpr int MAX_GRASS_WIDTH = 4;
    constexpr int MIN_ROAD_WIDTH = 1;
    constexpr int MAX_ROAD_WIDTH = 3;
    constexpr int MIN_RAIL_WIDTH = 1;
    constexpr int MAX_RAIL_WIDTH = 2;
    constexpr int MIN_RIVER_WIDTH = 1;
    constexpr int MAX_RIVER_WIDTH = 3;

    // --- Camera / Infinite World Tuning ---
    // Camera target slowly advances forward even if the player stalls.
    constexpr float CAMERA_AUTO_SCROLL_SPEED = 0.75f;          // world units per second
    constexpr float CAMERA_BACKWARD_DEATH_DISTANCE = 5.0f;    // allowed gap behind camera target
    constexpr float LANE_GENERATION_BUFFER_AHEAD = 25.0f;     // keep this much world ahead generated
    constexpr float LANE_CLEANUP_BUFFER_BEHIND = 12.0f;       // remove lanes this far behind player
    constexpr float CAMERA_SMOOTH_SPEED_XY = 1.0f; // Higher = faster catch-up on X/Y axis
    constexpr float CAMERA_SMOOTH_SPEED_Z  = 1.0f;  // Lower = softer catch-up on forward movement

    // --- Obstacle Config ---
    constexpr float CAR_SPEED_MIN = 3.0f;
    constexpr float CAR_SPEED_MAX = 7.0f;

    constexpr float TRAIN_SPEED = 25.0f; // Very fast
    constexpr float TRAIN_LENGTH = 15.0f; // Very long model/hitbox

    constexpr float LOG_SPEED_MIN = 2.0f;
    constexpr float LOG_SPEED_MAX = 4.0f;
}

#endif
