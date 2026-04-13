#ifndef TYPES_H
#define TYPES_H

// --- Game Enums ---
enum GameState { GAME_STATE_START_SCREEN, GAME_STATE_PLAYING, GAME_STATE_GAME_OVER };
enum LaneType  { LANE_GRASS, LANE_ROAD, LANE_RAIL, LANE_RIVER, LANE_LILYPAD }; // Added LILYPAD
enum ObstacleType { OBSTACLE_CAR, OBSTACLE_TRAIN, OBSTACLE_LOG, OBSTACLE_LILYPAD }; // Added LILYPAD

enum VehicleVariant { VEHICLE_SMALL_CAR, VEHICLE_BIG_CAR, VEHICLE_TRUCK };

// =============================================================
//  HYPERPARAMETERS  –  tweak everything from here
// =============================================================
namespace Config {

    // ── World ─────────────────────────────────────────────────
    constexpr float CELL_SIZE = 1.0f;
    constexpr int   INITIAL_SAFE_ZONE_LENGTH = 15;
    constexpr float HITBOX_PADDING           = 0.8f;

    // ── Lane generation ───────────────────────────────────────
    constexpr int MIN_GRASS_WIDTH = 2;  constexpr int MAX_GRASS_WIDTH = 4;
    constexpr int MIN_ROAD_WIDTH  = 1;  constexpr int MAX_ROAD_WIDTH  = 3;
    constexpr int MIN_RAIL_WIDTH  = 1;  constexpr int MAX_RAIL_WIDTH  = 2;
    constexpr int MIN_RIVER_WIDTH = 1;  constexpr int MAX_RIVER_WIDTH = 3;

    // ── Camera ────────────────────────────────────────────────
    constexpr float CAMERA_AUTO_SCROLL_SPEED        = 0.0f;
    constexpr float CAMERA_BACKWARD_DEATH_DISTANCE  = 5.0f;
    constexpr float LANE_GENERATION_BUFFER_AHEAD    = 25.0f;
    constexpr float LANE_CLEANUP_BUFFER_BEHIND      = 12.0f;
    constexpr float CAMERA_SMOOTH_SPEED_XY          = 1.0f;
    constexpr float CAMERA_SMOOTH_SPEED_Z           = 1.0f;

    // ── Cars / Trucks ─────────────────────────────────────────
    constexpr float CAR_SPEED_MIN = 2.0f;
    constexpr float CAR_SPEED_MAX = 4.0f;

    // ── Train ────────────────────────────────────────────────
    constexpr float TRAIN_SPEED  = 25.0f;
    constexpr float TRAIN_LENGTH = 15.0f;

    // ── Logs ─────────────────────────────────────────────────
    constexpr float LOG_SPEED_MIN   = 0.75f;
    constexpr float LOG_SPEED_MAX   = 1.5f;
    constexpr int   LOG_COUNT_MIN   = 2;      // logs spawned per river lane
    constexpr int   LOG_COUNT_MAX   = 3;
    constexpr float LOG_SPACING     = 4.5f;   // X distance between logs
    constexpr float LOG_WIDTH       = 2.8f;   // log length (X axis)
    constexpr float LOG_DEPTH       = 0.75f;  // log width  (Z axis)
    constexpr float LOG_HEIGHT      = 0.28f;  // log thickness (Y axis)
    constexpr float LOG_Y           = 0.14f;  // resting Y centre above ground

    // ── Log sinking (chicken riding effect) ──────────────────
    constexpr float LOG_SINK_AMOUNT = 0.07f;  // how far down the log sinks
    constexpr float LOG_SINK_SPEED  = 6.0f;   // lerp speed toward/away from sink

    // ── Lilypads ─────────────────────────────────────────────
    constexpr float LILYPAD_SIZE    = 0.9f;   // Size of the voxel footprint
    constexpr float LILYPAD_HEIGHT  = 0.15f;  // Thickness
    constexpr float LILYPAD_Y       = -0.05f; // Resting Y above water
    constexpr float LILYPAD_GAP_MIN = 1.2f;   // Minimum space between lilypads
    constexpr float LILYPAD_GAP_MAX = 50.0f;   // Maximum space between lilypads


    constexpr float EGG_SIZE = 0.35f;
    constexpr int   MAX_EGG_CLICKS   = 3;
    constexpr float DEAD_ZOOM_RADIUS = 4.0f;
    constexpr float DEAD_ZOOM_SPEED  = 3.0f;
}

#endif