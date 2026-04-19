#ifndef TYPES_H
#define TYPES_H

#include <glm/glm.hpp>

// --- Game Enums ---
// ADDED: GAME_STATE_MAIN_MENU and GAME_STATE_CHARACTER_SELECT
enum GameState { 
    GAME_STATE_MAIN_MENU, 
    GAME_STATE_CHARACTER_SELECT, 
    GAME_STATE_START_SCREEN, 
    GAME_STATE_PLAYING, 
    GAME_STATE_GAME_OVER 
};

enum LaneType  { LANE_GRASS, LANE_ROAD, LANE_RAIL, LANE_RIVER, LANE_LILYPAD };
enum ObstacleType { OBSTACLE_CAR, OBSTACLE_TRAIN, OBSTACLE_LOG, OBSTACLE_LILYPAD };

enum VehicleVariant { VEHICLE_SMALL_CAR, VEHICLE_BIG_CAR, VEHICLE_TRUCK };

// =============================================================
//  HYPERPARAMETERS  –  tweak everything from here
// =============================================================
namespace Config {

    // ── World ─────────────────────────────────────────────────
    constexpr float CELL_SIZE = 1.0f;
    constexpr int   INITIAL_SAFE_ZONE_LENGTH = 15;
    constexpr float HITBOX_PADDING           = 0.8f;

    // ── Boundaries ────────────────────────────────────────────
    // Player movement is blocked at ±BOUNDARY_X (mountain wall starts just beyond)
    constexpr float BOUNDARY_X          = 13.5f;
    // Mountain cliff inner face sits at ±MOUNTAIN_WALL_X
    constexpr float MOUNTAIN_WALL_X     = 14.5f;
    // How many columns the mountain extends outward from its inner face
    constexpr int   MOUNTAIN_COLS       = 8;
    // Back wall z (player can't move past this positive Z)
    constexpr float BOUNDARY_BACK_Z     = 2.0f;

    // ── Lane generation ───────────────────────────────────────
    constexpr int MIN_GRASS_WIDTH = 1;  constexpr int MAX_GRASS_WIDTH = 3;
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
    constexpr int   LOG_COUNT_MIN   = 2;
    constexpr int   LOG_COUNT_MAX   = 3;
    constexpr float LOG_SPACING     = 4.0f;
    constexpr float LOG_WIDTH       = 2.8f;
    constexpr float LOG_DEPTH       = 0.75f;
    constexpr float LOG_HEIGHT      = 0.28f;
    constexpr float LOG_Y           = 0.14f;
    constexpr int LOG_SETS = 2; 
    constexpr float LOG_SET_GAP = 15.0f;

    // ── Log sinking ───────────────────────────────────────────
    constexpr float LOG_SINK_AMOUNT = 0.07f;
    constexpr float LOG_SINK_SPEED  = 6.0f;

    // ── Fast-stream (log hits boundary foam) ─────────────────
    // When log abs(x) exceeds this, it enters fast-stream mode
    constexpr float LOG_STREAM_TRIGGER_X = 12.0f;
    // Speed multiplier applied per-second in stream zone (exponential)
    constexpr float LOG_STREAM_ACCEL     = 6.0f;
    // Cap speed in fast-stream
    constexpr float LOG_STREAM_MAX_SPEED = 22.0f;
    // No-wrap threshold: logs in fast-stream don't wrap until this x
    constexpr float LOG_STREAM_EXIT_X    = 20.0f;

    // ── Lilypads ─────────────────────────────────────────────
    constexpr float LILYPAD_SIZE    = 0.9f;
    constexpr float LILYPAD_HEIGHT  = 0.15f;
    constexpr float LILYPAD_Y       = -0.05f;
    constexpr float LILYPAD_GAP_MIN = 1.2f;
    constexpr float LILYPAD_GAP_MAX = 50.0f;

    constexpr float EGG_SIZE = 0.35f;
    constexpr int   MAX_EGG_CLICKS   = 3;
    constexpr float DEAD_ZOOM_RADIUS = 4.0f;
    constexpr float DEAD_ZOOM_SPEED  = 3.0f;

    // ── Signal Posts ─────────────────────────────────────────
    constexpr float SIGNAL_SPACING = 8.0f;   // base interval between posts
    constexpr float SIGNAL_RANGE   = 20.0f;  // ±range from centre
    constexpr float SIGNAL_Z_SIDE  = 0.42f;  // visual offset to lane near-edge
    constexpr float SIGNAL_JITTER  = 1.4f;   // max random X nudge per post

    // ── Day/Night Cycle & Shadows ────────────────────────────
    constexpr float TIME_SPEED = 0.01f;           // rate of day/night cycle (seconds/cycle)
    constexpr float TRANSITION_SMOOTHNESS = 4.0f; // duration of day/night blend (seconds)
    constexpr float TRANSITION_ZONE_WIDTH = 0.20f; // width of transition window around horizon (radians)
    constexpr float SUN_LIGHT_INTENSITY = 1.2f;   // sun brightness multiplier
    constexpr float MOON_LIGHT_INTENSITY = 0.5f;  // moon brightness multiplier
    
    // Light direction: sun moves left to right across sky
    // This vector represents the direction the sun travels (e.g., +X direction)
    constexpr glm::vec3 LIGHT_DIRECTION_VECTOR = glm::vec3(1.0f, 1.0f, 0.0f);
    
    // Shadow rendering parameters
    constexpr float SHADOW_Y_OFFSET = 0.02f;     // height above ground to prevent z-fighting (tweakable)
    constexpr float SHADOW_Z_OFFSET = -0.1f;      // z-axis offset for shadows (0 = no offset between lanes)
    constexpr float SHADOW_OPACITY = 0.4f;       // alpha/opacity of shadows
    constexpr float SHADOW_MAX_LENGTH = 3.0f;    // maximum shadow projection length
    constexpr float SHADOW_MIN_LENGTH = 0.5f;    // minimum shadow length (when sun is overhead)
    
    // Shadow fade transition parameters
    constexpr float SHADOW_FADE_START_ANGLE = 0.524f;  // angle threshold for fade start (≈30° = π/6, before horizon)
    constexpr float SHADOW_FADE_DURATION = 0.5f;       // duration of fade in/out at transition (seconds equivalent)
    
    // Lane-specific shadow heights (Y-coordinate for shadow rendering)
    constexpr float SHADOW_HEIGHT_ROAD = 0.10f;     // shadow height on road lanes (well above surface for visibility)
    constexpr float SHADOW_HEIGHT_GRASS = 0.10f;    // shadow height on grass lanes
    constexpr float SHADOW_HEIGHT_RAIL = 0.10f;     // shadow height on rail lanes
    constexpr float SHADOW_HEIGHT_RIVER = -0.1f;   // shadow height on river lanes (lower)
    constexpr float SHADOW_HEIGHT_LILYPAD = -0.1f; // shadow height on lilypad lanes
}

// Helper function for random float generation (more efficient than repeated casting)
inline float randomRange(float min, float max) {
    return min + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min);
}

#endif