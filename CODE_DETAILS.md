# Crazy Hopper Game - Detailed Code Analysis
## Complete In-Depth Breakdown of All Game Files

This document provides a comprehensive analysis of every file in the Crazy Hopper graphics project, explaining code piece by piece with detailed explanations.

---

## Table of Contents
1. [Core System Files](#core-system-files)
2. [Game Engine Files](#game-engine-files)
3. [Character & Animation Files](#character--animation-files)
4. [Gameplay Mechanics](#gameplay-mechanics)
5. [Rendering & Graphics](#rendering--graphics)

---

# CORE SYSTEM FILES

## 1. main.cpp - Entry Point & GLUT Event Handlers

### Global Variables Setup

```cpp
#include <GL/glew.h>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#include "game.h"
#include "types.h"
#include "save_data.h"

#include <iostream>
#include <chrono>

Game* game = nullptr;
```

**Explanation:** This section includes platform-specific headers for OpenGL rendering:
- `GLEW.h` provides OpenGL function pointers across different platforms
- Conditional compilation handles macOS (GLUT) vs Linux (GL/glut.h) differences
- Global `game` pointer holds the main Game object used by all GLUT callbacks
- Headers for game logic, type definitions, and save data management are included

### Mouse Tracking Variables

```cpp
bool isDragging = false;
int lastMouseX = -1;
int lastMouseY = -1;
```

**Explanation:** These track mouse drag state for camera control:
- `isDragging`: Boolean flag indicating if left mouse button is currently pressed
- `lastMouseX/Y`: Previous mouse position to calculate delta movement for camera rotation

### Frame Rate Limiting

```cpp
auto lastFrameTime = std::chrono::high_resolution_clock::now();
```

**Explanation:** Uses `chrono` library for high-resolution timing to implement frame rate limiting and ensure consistent game speed across different machines.

### Display Callback

```cpp
void display() {
    game->render();
    glutSwapBuffers();
}
```

**Detailed Explanation - Rendering Pipeline Callback:**

**Algorithm Type**: Double-buffered rendering callback

**Purpose**: GLUT calls this whenever screen needs redrawing (triggered by glutPostRedisplay())

**Step 1: Execute All Game Rendering**
```cpp
game->render();
```

**Render Pipeline (in order):**
- Clear framebuffer (erase previous frame)
- Set camera view matrix (gluLookAt from player position)
- Render shadow geometry (darkened ground under objects)
- Render lane ground (grass, road, water, rails)
- Render obstacles (trains, cars, logs, lilypads)
- Render coins (spinning 3D circles)
- Render character (player model at current position)
- Render particles (water death splash effects)
- Apply day/night cycle (sun angle affects lighting)
- Render UI overlay (2D: score, HUD, buttons)

**Step 2: Swap Display Buffers**
```cpp
glutSwapBuffers();
```

**Double Buffering Technique - Why It Matters:**

```
Without double buffering (single buffer, OLD approach):
  Frame 1: Draw directly to screen display memory
           Player sees partial renders: flickering, tearing
  
With double buffering (current GPU standard):
  Memory Layout:
    Front Buffer: Currently displayed on screen (shown to user)
    Back Buffer:  Invisible, being drawn to (GPU writes here)
  
  Frame 1 (t=0s):
    1. GPU renders entire scene to BACK buffer
       Screen STILL shows previous frame from FRONT buffer
    2. glutSwapBuffers() called
       ATOMIC SWAP: Front ↔ Back (no tearing possible)
       Screen NOW shows NEW frame, completely rendered
    3. Old FRONT becomes new BACK buffer ready for frame 2
  
  Result: Screen always displays complete, finished frames
          No flickering, no tearing, smooth animation
```

**Swap Synchronization with Monitor:**
```
Monitor refresh: 60 Hz = redraw screen every 16.67ms

Without vsync (fast but tearing):
  Swap at random time → screen mid-refresh shows old top + new bottom
  
With vsync (smooth but potentially delayed):
  GPU waits for vertical blank (beam returns to top)
  Then swaps buffers
  Next scan starts with new frame buffer
  Result: No tearing, but ~16.67ms delay possible
  
This game: Software frame rate limiting (120 FPS target via chrono timing)
  Note: NOT hardware vsync - uses fixed timestep instead for cross-platform consistency
```

**Frame Timing Analysis:**
```
120 FPS target:
  Frame time: 1/120 = 8.33ms per frame
  
  t=0ms:   idle() checks if time to update
  t=0-8ms: Game logic runs (physics, collision, input)
  t=8ms:   glutPostRedisplay() called
  t=9ms:   GLUT calls display() callback
  t=9ms:   game->render() executes
             - Clear framebuffers
             - Draw 3D world
             - Draw UI
             - ~5-8ms of GPU work
  t=14-16ms: glutSwapBuffers() calls
             - Swaps Front ↔ Back buffers
             - Screen updates with new frame
             - May experience tearing if not vsync'd to monitor
  t=16ms:  Frame complete, cycle repeats
```
**Purpose**: GLUT calls this whenever screen needs redrawing (triggered by glutPostRedisplay())

**Step 1: Execute All Game Rendering**
```cpp
game->render();
```

**Render Pipeline (in order):**
- Clear framebuffer (erase previous frame)
- Set camera view matrix (gluLookAt from player position)
- Render shadow geometry (darkened ground under objects)
- Render lane ground (grass, road, water, rails)
- Render obstacles (trains, cars, logs, lilypads)
- Render coins (spinning 3D circles)
- Render character (player model at current position)
- Render particles (water death splash effects)
- Apply day/night cycle (sun angle affects lighting)
- Render UI overlay (2D: score, HUD, buttons)

**Step 2: Swap Display Buffers**
```cpp
glutSwapBuffers();
```

**Double Buffering Technique - Why It Matters:**

```
Without double buffering (single buffer, OLD approach):
  Frame 1: Draw directly to screen display memory
           Player sees partial renders: flickering, tearing
  
With double buffering (current GPU standard):
  Memory Layout:
    Front Buffer: Currently displayed on screen (shown to user)
    Back Buffer:  Invisible, being drawn to (GPU writes here)
  
  Frame 1 (t=0s):
    1. GPU renders entire scene to BACK buffer
       Screen STILL shows previous frame from FRONT buffer
    2. glutSwapBuffers() called
       ATOMIC SWAP: Front ↔ Back (no tearing possible)
       Screen NOW shows NEW frame, completely rendered
    3. Old FRONT becomes new BACK buffer ready for frame 2
  
  Result: Screen always displays complete, finished frames
          No flickering, no tearing, smooth animation
```

**Swap Synchronization with Monitor:**
```
Monitor refresh: 60 Hz = redraw screen every 16.67ms

Without vsync (fast but tearing):
  Swap at random time → screen mid-refresh shows old top + new bottom
  
With vsync (smooth but potentially delayed):
  GPU waits for vertical blank (beam returns to top)
  Then swaps buffers
  Next scan starts with new frame buffer
  Result: No tearing, but ~16.67ms delay possible
  
This game: Software frame rate limiting (120 FPS target via chrono timing)
  Note: NOT hardware vsync - uses fixed timestep instead for cross-platform consistency
```

**Frame Timing Analysis:**
```
120 FPS target:
  Frame time: 1/120 = 8.33ms per frame
  
  t=0ms:   idle() checks if time to update
  t=0-8ms: Game logic runs (physics, collision, input)
  t=8ms:   glutPostRedisplay() called
  t=9ms:   GLUT calls display() callback
  t=9ms:   game->render() executes
             - Clear framebuffers
             - Draw 3D world
             - Draw UI
             - ~5-8ms of GPU work
  t=14-16ms: glutSwapBuffers() calls
             - Swaps Front ↔ Back buffers
             - Screen updates with new frame
             - May experience tearing if not vsync'd to monitor
  t=16ms:  Frame complete, cycle repeats
```

### Idle Callback - Main Game Loop

```cpp
void idle() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - lastFrameTime;
    
    if (elapsed.count() >= Config::FRAME_TIME) {
        game->update(elapsed.count() * Config::GAME_SPEED_MULTIPLIER);
        lastFrameTime = currentTime;
        glutPostRedisplay();
    }
}
```

**Detailed Explanation - Frame Rate Limiting Algorithm:**

**Algorithm Type**: Fixed timestep with delta-time accumulation

**Input Parameters:**
- `lastFrameTime`: Global variable storing previous frame's timestamp (type: `std::chrono::high_resolution_clock::time_point`)
- `Config::FRAME_TIME`: Target frame interval = 1/120 = 0.00833... seconds
- `Config::GAME_SPEED_MULTIPLIER`: 2.6x (speeds up game logic relative to real time)

**Execution Flow:**

**Step 1: Capture Current Time**
```cpp
auto currentTime = std::chrono::high_resolution_clock::now()
```
- Gets nanosecond-precision timestamp from OS
- `high_resolution_clock::now()` returns current wall-clock time
- Resolution: typically nanosecond level on modern systems
- Returns: `time_point` object representing exact moment

**Step 2: Calculate Elapsed Duration**
```cpp
std::chrono::duration<float> elapsed = currentTime - lastFrameTime
```
- Subtracts two `time_point` objects to get `duration`
- Result templated as `float` (duration in floating-point seconds)
- `duration.count()` extracts numeric value (e.g., 0.008333f)
- Math: If `lastFrameTime` was 100.0s and `currentTime` is 100.008333s, then `elapsed = 0.008333s`

**Step 3: Frame Rate Gating Decision**
```cpp
if (elapsed.count() >= Config::FRAME_TIME)
```
- **Check**: Has enough time passed for next update?
- `Config::FRAME_TIME` = 0.008333s (1/120 second)
- **Condition**: Only proceed if elapsed ≥ 8.33 milliseconds
- **Why**: CPU-bound frame limiting. Without this:
  - Loop would run thousands of times per second (busy loop)
  - Game logic would update erratically
  - Physics would be unrealistic (dt would vary wildly)

**Concrete Example of Frame Rate Limiting:**
Detailed Explanation - Keyboard Input Event Routing:**

**Algorithm Type**: Event-driven callback with early exit and game state dispatch

**Inputs:**
- `key`: Integer key code (ASCII for letters/numbers, special values for function keys)
- `x, y`: Mouse position when key pressed (unused in this game)

**Step 1: Check for Exit Signal**
```cpp
if (key == 27) { // ESC = ASCII Escape character
    delete game;  // Cleanup
    exit(0);      // Terminate program
```

**Key Code Reference:**
```
ASCII codes:
  27 = ESC
  13 = Enter/Return
  32 = Space
  65-90 = A-Z (uppercase)
  97-122 = a-z (lowercase)
  48-57 = 0-9
```

**Cleanup on Exit:**
```cpp
delete game;  // Destructor called
```
- Frees all Game object memory
- Shaders deleted (glDeleteProgram)
- Textures deleted (glDeleteTextures)
- VBOs deleted (glDeleteBuffers)
- Calls SaveManager to persist game state
- Closes file handles

**Process Termination:**
```cpp
exit(0);
```
- Kills entire program
- Return code 0 = success (shell gets this code)
- GLUT event loop breaks immediately
- OS reclaims all resources

**Why Early Exit Pattern:**
- ESC is universal "quit" convention
- Player expects instant exit
- Alternative: glutLeaveMainLoop() in GLUT 3.0+, but older GLUT needed

**Step 2: Route to Game Logic**
```cpp
game->onKeyPress(key);
```

**Game State Dispatch (in onKeyPress):**

```
switch (gameState) {
  case GAME_STATE_MAIN_MENU:
    if (key=='1') selectCharacter(CHICKEN);
    if (key=='2') selectCharacter(FROG);
    if (key=='3') selectCharacter(DINO);
    if (key=='4') selectCharacter(CAT);
    if (key=='5') selectCharacter(DOG);
    if (key==13) transitionTo(CHARACTER_SELECT);  // Enter
    break;
    
  case GAME_STATE_PLAYING:
    if (key=='w' || key=='W') jumpForward();
    if (key=='a' || key=='A') jumpLeft();
    if (key=='s' || key=='S') jumpBackward();
    if (key=='d' || key=='D') jumpRight();
    break;
    
  case GAME_STATE_GAME_OVER:
    if (key=='r' || key=='R') resetGame();
    if (key=='m' || key=='M') showMainMenu();
    break;
}
```

**Jump Input Example (Playing State):**
```
User presses: 'W' key

Callstack:
  1. OS keyboard event → keyboard(87, x, y)  // 87=ASCII 'W'
  2. keyboard() checks: is 27 (ESC)? No
  3. Calls game->onKeyPress(87)
  4. Game: state is PLAYING? Check
  5. Is 'w' or 'W'? Check (87 matches)
  6. isJumping already? No
  7. Call jumpForward():
     - Calculate target position forward
     - Set isJumping = true
     - Store start parameters (position, time)
  8. Return to keyboard callback
  9. GLUT continues event loop
  10. Next idle(): update() called
  11. Character animation advances jump arc
  12. Next display(): renders at new position
```

**Step 3: Special Keys Callback**
```cpp
void specialKey(int key, int x, int y) {
    game->onSpecialKey(key);
}
```

**Why Separate Callback:**
```
GLUT provides TWO keyboard callbacks:
  glutKeyboardFunc()  → Regular ASCII keys
  glutSpecialFunc()   → Arrow keys, Function keys, etc.
  
Reason: Special keys handled differently by OS
  - Windows/Linux/Mac each encode differently
  - GLUT abstracts platform differences
  - Separate codes prevent conflicts
  
Example:
  'W' ASCII = 87 (same on all platforms)
  Up Arrow = GLUT_KEY_UP (special GLUT constant, not ASCII)
```

**Special Key Codes:**
```
GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_LEFT, GLUT_KEY_RIGHT
GLUT_KEY_F1 through GLUT_KEY_F12
GLUT_KEY_PAGE_UP, GLUT_KEY_PAGE_DOWN
GLUT_KEY_HOME, GLUT_KEY_END
```

**In This Game:**
- Arrow keys same as WASD (jump inputs)
- May use F keys for camera presets or debug info

**Input Priority (Typical Game):**
```
If both 'W' and Arrow Up pressed:
  Only one fires per frame (depends on OS event ordering)
  Usually: last key processed wins
  Most games accept both for redundancy
```
Time 0.002s:     idle() called, currentTime=0.002, elapsed=0.002 → SKIP
Time 0.003s:     idle() called, currentTime=0.003, elapsed=0.003 → SKIP
Time 0.004s:     idle() called, currentTime=0.004, elapsed=0.004 → SKIP
Time 0.005s:     idle() called, currentTime=0.005, elapsed=0.005 → SKIP
Time 0.006s:     idle() called, currentTime=0.006, elapsed=0.006 → SKIP
Time 0.007s:     idle() called, currentTime=0.007, elapsed=0.007 → SKIP
Time 0.008333s:  idle() called, currentTime=0.008333, elapsed=0.008333 → PASS ✓ (0.008333 >= 0.008333)
            [UPDATE GAME WITH dt=0.008333 * 2.6 = 0.0216]
            [lastFrameTime = 0.008333]
Time 0.009s:     idle() called, currentTime=0.009, elapsed=0.000667 → SKIP
Time 0.010s:     idle() called, currentTime=0.010, elapsed=0.001667 → SKIP
...continuing pattern...
Time 0.016666s:  idle() called, currentTime=0.016666, elapsed=0.008333 → PASS ✓
            [UPDATE GAME WITH dt=0.008333 * 2.6 = 0.0216]
            [lastFrameTime = 0.016666]
```

**Result**: Game updates at exactly 120 Hz even though `idle()` called thousands of times/second

**Step 4: Update Game State**
```cpp
gaDetailed Explanation - Mouse Button & Drag State Management:**

**Algorithm Type**: Multi-dispatch event handler with state machine

**Inputs:**
- `button`: Which button (GLUT_LEFT_BUTTON=0, GLUT_MIDDLE_BUTTON=1, GLUT_RIGHT_BUTTON=2)
- `state`: Press or release (GLUT_DOWN=0, GLUT_UP=1)
- `x, y`: Screen pixel coordinates (0,0 at top-left)

**Step 1: Null Check Game Pointer**
```cpp
if (game != nullptr) {
    game->onMouseClick(button, state, x, y);
}
```

**Safety Check Purpose:**
- Game might be nullptr during initialization phase
- Prevents crash if mouse clicked during startup
- GLUT might call callback before game fully constructed

**UI Button Click Handling (onMouseClick):**
```
Clicks route to game state:

MAIN_MENU:
  Click on "PLAY" button region → transitionTo(START_SCREEN)
  Click on "CHARACTERS" button → transitionTo(CHARACTER_SELECT)
  
CHARACTER_SELECT:
  Click on left/right arrow circles → cycleCharacter()
  Click on "SELECT!" button → selectCharacter() and return to MAIN_MENU
  Click on "< BACK" button → return to MAIN_MENU
  
START_SCREEN:
  Click anywhere on egg → increment eggClicks
  After 3 egg clicks → transitionTo(PLAYING)
  
GAME_OVER:
  Click on "TRY AGAIN" button → return to MAIN_MENU (triggers reset)

Hit Testing (Bounding Box Check):
  Button regions defined as circles/rectangles with exact coordinates:
    PLAY button: centered, pulsing animation
    CHARACTERS button: x:[13,131], invertedY:[31,73]
    SELECT! button: centered at cy-148 with 100 width, 56 height
    < BACK button: x:[13,97], invertedY:[windowHeight-67, windowHeight-33]
    TRY AGAIN button: centered at cy-98 with 230 width, 62 height
    Arrow buttons (LEFT/RIGHT): circular, x=52 and x=windowWidth-52, radius=38
  
  Hit test: If click falls within button bounds/circle, trigger action
```

**Step 2: Track Left Mouse Button for Camera Drag**
```cpp
if (button == GLUT_LEFT_BUTTON) {
```

**Button Selection:**
```
GLUT_LEFT_BUTTON (0)    → Primary input (camera rotation)
GLUT_MIDDLE_BUTTON (1)  → Secondary (could pan camera)
GLUT_RIGHT_BUTTON (2)   → Context menu (unused here)

This game only responds to left button
```

**Step 2a: Drag Initiation (Button Pressed)**
```cpp
if (state == GLUT_DOWN) {
    isDragging = true;
    lastMouseX = x;
    lastMouseY = y;
```

**Drag Start Sequence:**
```
Event: Left mouse button pressed at screen (450, 300)

Execution:
  isDragging = true     (enter drag mode)
  lastMouseX = 450      (baseline X position)
  lastMouseY = 300      (baseline Y position)
  
Result: Ready to calculate delta on next mouse motion
```

**Why Store Baseline:**
- Every pixel moved calculates delta from baseline
- Delta (currentX - lastMouseX) = mouse displacement
- Used to convert pixels to camera rotation angles

**Example Sequence:**
```
t=0ms:   Mouse button down at (400, 300)
         isDragging=true, lastMouseX=400, lastMouseY=300
         
t=16ms:  Mouse motion event at (450, 300)
         In mouseMotion callback:
           deltaX = 450 - 400 = 50 pixels right
           camera rotates ~0.5 radians right
           lastMouseX = 450 (update baseline)
           
t=32ms:  Mouse motion at (470, 350)
         In mouseMotion:
           deltaX = 470 - 450 = 20 pixels
           deltaY = 350 - 300 = 50 pixels down
           camera rotates 20px right + 50px down
           lastMouseX = 470, lastMouseY = 350
           
t=50ms:  Mouse button up
         isDragging=false (exit drag mode)
         Camera stops responding to mouse movement
```

**Step 2b: Drag Termination (Button Released)**
```cpp
} else if (state == GLUT_UP) {
    isDragging = false;
```

**Drag End:**
```
When left button released:
  isDragging = false (exit drag mode)
  
Result: mouseMotion() will skip processing (isDragging check fails)
        Camera no longer rotates with mouse movement
```

### Mouse Motion Callback

```cpp
void mouseMotion(int x, int y) {
    if (isDragging) {
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;
        lastMouseX = x;
        lastMouseY = y;
        camera->processMouseDrag(deltaX, deltaY);
    }
}
```

**Detailed Explanation - Continuous Mouse Drag Processing:**

**Algorithm Type**: Differential tracking with state-gated processing

**Purpose**: Convert real-time mouse movement to camera rotation

**Step 1: Guard Check**
```cpp
if (isDragging) {
```

**State Gate:**
```
If isDragging = false:
  Function returns immediately (even if mouse moves)
  Camera unresponsive
  
If isDragging = true:
  Process mouse movement into camera rotation
  
Use Case: Player can move mouse freely without rotating camera
```

**Step 2: Calculate Delta Movement**
```cpp
int deltaX = x - lastMouseX;
int deltaY = y - lastMouseY;
```

**Example Timeline (User Drags Mouse Right):**
```
Frame 1: lastMouseX=400, current x=450
         deltaX = 450 - 400 = +50 pixels (dragged right)
         
Frame 2: lastMouseX=450, current x=470
         deltaX = 470 - 450 = +20 pixels (slower drag right)
         
Frame 3: lastMouseX=470, current x=460
         deltaX = 460 - 470 = -10 pixels (dragged LEFT!)
         
Frame 4: lastMouseX=460, current x=460
         deltaX = 0 (no movement)
```

**Coordinate Spaces:**
```
Screen coordinates: (0,0) at top-left
  X: 0 (left) → 800 (right)
  Y: 0 (top) → 600 (bottom)
  
Delta space: difference in pixels
  +deltaX: right movement
  -deltaX: left movement
  +deltaY: down movement
  -deltaY: up movement
```

**Step 3: Update Baseline for Next Frame**
```cpp
lastMouseX = x;
lastMouseY = y;
```

**Continuous Tracking:**
- After calculating delta, move baseline to current position
- Next frame: current position becomes previous position
- Creates seamless tracking across all frames

**Step 4: Pass to Camera**
```cpp
camera->processMouseDrag(deltaX, deltaY);
```

**Camera Processing:**
```
Inside Camera::processMouseDrag(deltaX, deltaY):
  targetYaw += deltaX * mouseSensitivity;
  targetPitch += deltaY * mouseSensitivity;
  
  With mouseSensitivity = 0.01:
    +50 pixels → +0.5 radians yaw (~29 degrees)
    +20 pixels → +0.2 radians pitch (~11 degrees)
  
  Camera then lerps smoothly to new target over ~0.3 seconds
  (exponential lerp at 7.0x speed from earlier expansion)
```

**Latency Analysis:**
```
User moves mouse:
  t=0ms:     Physical mouse moved
  t=1ms:     OS detects motion
  t=5ms:     GLUT polls mouse position
  t=6ms:     mouseMotion() callback invoked
  t=7ms:     camera->processMouseDrag() updates target
  t=8ms:     Next idle() call
  t=8.33ms:  camera update lerps toward target
  t=16.67ms: display() renders at lerped position
  
Total: 8-16ms (acceptable for game, ~1 frame latency)
```

**GLUT Callback Registration:**
```cpp
glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    // NOT glutPassiveMotionFunc (button required, not passive)
```

**Key GLUT Distinction:**
```
glutMotionFunc(mouseMotion):
  Called when MOUSE BUTTON HELD + MOUSE MOVES
  Used for: drag operations, camera control
  
glutPassiveMotionFunc():
  Called when MOUSE MOVES (NO buttons held)
  Used for: hover effects, tooltips
  Not used in this game
```
  - `elapsed.count()` = actual elapsed seconds (e.g., 0.008333)
  - Multiply by `2.6x` game speed multiplier
  - Result: 0.008333 × 2.6 = 0.0216 seconds "game time"
- **Why Multiply**: Makes game logic run 2.6× faster than real-time
  - Player feels like faster-paced game
  - All physics, animations, obstacle movement use this dt
  - Camera follows at adjusted speed
- **passes to `game->update()`**:
  - All game logic runs with this modified dt
  - Character jumps happen in scaled time
  - Obstacles move at scaled speeds
  - Even though actual frame time is 0.008333s, game thinks 0.0216s passed

**Step 5: Update Timestamp & Redraw Request**
```cpp
lastFrameTime = currentTime
glutPostRedisplay()
```
- **Update Tracking**: Save current time as baseline for next frame's elapsed calculation
- **Redraw Request**: Tell GLUT to call `display()` callback at next opportunity
  - `glutPostRedisplay()` just sets a flag in GLUT
  - Actual rendering happens asynchronously
  - Allows multiple game updates before display (if updates very fast)

**Why This Approach (Adaptive Fixed Timestep):**

| Approach | Pros | Cons |
|----------|------|------|
| **Fixed timestep (this one)** | Deterministic, predictable, stable physics | Skip render frames if too slow |
| | Game speed same on all hardware | Wasted CPU if running > 120 FPS capable |
| **Variable timestep** | Smooth to fast hardware, responsive | Physics unstable, depends on framerate |
| | No wasted cycles | Different experience on different PCs |
| **Vsync locked** | Smooth, power-efficient | Locked to monitor refresh (60Hz/144Hz varies) |

**Performance Impact:**
- CPU-bound limiting saves power (no 1000s of idle() calls updating nothing)
- Smoothes out OS scheduling jitter
- Ensures consistent gameplay experience across devices

### Keyboard Input

```cpp
void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // ESC
        delete game;
        exit(0);
    }
    game->onKeyPress(key);
}

void specialKey(int key, int x, int y) {
    game->onSpecialKey(key);
}
```

**Detailed Explanation - Keyboard Input Event Routing:**

**Algorithm Type**: Event-driven callback with early exit and game state dispatch

**Inputs:**
- `key`: Integer key code (ASCII for letters/numbers, special values for function keys)
- `x, y`: Mouse position when key pressed (unused in this game)

**Step 1: Check for Exit Signal**
```cpp
if (key == 27) { // ESC = ASCII Escape character
    delete game;  // Cleanup
    exit(0);      // Terminate program
```

**Key Code Reference:**
```
ASCII codes:
  27 = ESC
  13 = Enter/Return
  32 = Space
  65-90 = A-Z (uppercase)
  97-122 = a-z (lowercase)
  48-57 = 0-9
```

**Cleanup on Exit:**
```cpp
delete game;  // Destructor called
```
- Frees all Game object memory
- Shaders deleted (glDeleteProgram)
- Textures deleted (glDeleteTextures)
- VBOs deleted (glDeleteBuffers)
- Calls SaveManager to persist game state
- Closes file handles

**Process Termination:**
```cpp
exit(0);
```
- Kills entire program
- Return code 0 = success (shell gets this code)
- GLUT event loop breaks immediately
- OS reclaims all resources

**Why Early Exit Pattern:**
- ESC is universal "quit" convention
- Player expects instant exit
- Alternative: glutLeaveMainLoop() in GLUT 3.0+, but older GLUT needed

**Step 2: Route to Game Logic**
```cpp
game->onKeyPress(key);
```

**Game State Dispatch (in onKeyPress):**

```
switch (gameState) {
  case GAME_STATE_MAIN_MENU:
    if (key=='1') selectCharacter(CHICKEN);
    if (key=='2') selectCharacter(FROG);
    if (key=='3') selectCharacter(DINO);
    if (key=='4') selectCharacter(CAT);
    if (key=='5') selectCharacter(DOG);
    if (key==13) transitionTo(CHARACTER_SELECT);  // Enter
    break;
    
  case GAME_STATE_PLAYING:
    if (key=='w' || key=='W') jumpForward();
    if (key=='a' || key=='A') jumpLeft();
    if (key=='s' || key=='S') jumpBackward();
    if (key=='d' || key=='D') jumpRight();
    break;
    
  case GAME_STATE_GAME_OVER:
    if (key=='r' || key=='R') resetGame();
    if (key=='m' || key=='M') showMainMenu();
    break;
}
```

**Jump Input Example (Playing State):**
```
User presses: 'W' key

Callstack:
  1. OS keyboard event → keyboard(87, x, y)  // 87=ASCII 'W'
  2. keyboard() checks: is 27 (ESC)? No
  3. Calls game->onKeyPress(87)
  4. Game: state is PLAYING? Check
  5. Is 'w' or 'W'? Check (87 matches)
  6. isJumping already? No
  7. Call jumpForward():
     - Calculate target position forward
     - Set isJumping = true
     - Store start parameters (position, time)
  8. Return to keyboard callback
  9. GLUT continues event loop
  10. Next idle(): update() called
  11. Character animation advances jump arc
  12. Next display(): renders at new position
```

**Step 3: Special Keys Callback**
```cpp
void specialKey(int key, int x, int y) {
    game->onSpecialKey(key);
}
```

**Why Separate Callback:**
```
GLUT provides TWO keyboard callbacks:
  glutKeyboardFunc()  → Regular ASCII keys
  glutSpecialFunc()   → Arrow keys, Function keys, etc.
  
Reason: Special keys handled differently by OS
  - Windows/Linux/Mac each encode differently
  - GLUT abstracts platform differences
  - Separate codes prevent conflicts
  
Example:
  'W' ASCII = 87 (same on all platforms)
  Up Arrow = GLUT_KEY_UP (special GLUT constant, not ASCII)
```

**Special Key Codes:**
```
GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_LEFT, GLUT_KEY_RIGHT
GLUT_KEY_F1 through GLUT_KEY_F12
GLUT_KEY_PAGE_UP, GLUT_KEY_PAGE_DOWN
GLUT_KEY_HOME, GLUT_KEY_END
```

**In This Game:**
- Arrow keys same as WASD (jump inputs)
- May use F keys for camera presets or debug info

**Input Priority (Typical Game):**
```
If both 'W' and Arrow Up pressed:
  Only one fires per frame (depends on OS event ordering)
  Usually: last key processed wins
  Most games accept both for redundancy
```

### Mouse Input Handling

```cpp
void mouseButton(int button, int state, int x, int y) {
    if (game != nullptr) {
        game->onMouseClick(button, state, x, y);
    }

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        } else if (state == GLUT_UP) {
            isDragging = false;
        }
    }
}
```

**Detailed Explanation - Mouse Button & Drag State Management:**

**Algorithm Type**: Multi-dispatch event handler with state machine

**Inputs:**
- `button`: Which button (GLUT_LEFT_BUTTON=0, GLUT_MIDDLE_BUTTON=1, GLUT_RIGHT_BUTTON=2)
- `state`: Press or release (GLUT_DOWN=0, GLUT_UP=1)
- `x, y`: Screen pixel coordinates (0,0 at top-left)

**Step 1: Null Check Game Pointer**
```cpp
if (game != nullptr) {
    game->onMouseClick(button, state, x, y);
}
```

**Safety Check Purpose:**
- Game might be nullptr during initialization phase
- Prevents crash if mouse clicked during startup
- GLUT might call callback before game fully constructed

**UI Button Click Handling (onMouseClick):**
```
Clicks route to game state:

MAIN_MENU:
  Click on "PLAY" button region → transitionTo(START_SCREEN)
  Click on "CHARACTERS" button → transitionTo(CHARACTER_SELECT)
  
CHARACTER_SELECT:
  Click on left/right arrow circles → cycleCharacter()
  Click on "SELECT!" button → selectCharacter() and return to MAIN_MENU
  Click on "< BACK" button → return to MAIN_MENU
  
START_SCREEN:
  Click anywhere on egg → increment eggClicks
  After 3 egg clicks → transitionTo(PLAYING)
  
GAME_OVER:
  Click on "TRY AGAIN" button → return to MAIN_MENU (triggers reset)

Hit Testing (Bounding Box Check):
  Button regions defined as circles/rectangles with exact coordinates:
    PLAY button: centered, pulsing animation
    CHARACTERS button: x:[13,131], invertedY:[31,73]
    SELECT! button: centered at cy-148 with 100 width, 56 height
    < BACK button: x:[13,97], invertedY:[windowHeight-67, windowHeight-33]
    TRY AGAIN button: centered at cy-98 with 230 width, 62 height
    Arrow buttons (LEFT/RIGHT): circular, x=52 and x=windowWidth-52, radius=38
  
  Hit test: If click falls within button bounds/circle, trigger action
```

**Step 2: Track Left Mouse Button for Camera Drag**
```cpp
if (button == GLUT_LEFT_BUTTON) {
```

**Button Selection:**
```
GLUT_LEFT_BUTTON (0)    → Primary input (camera rotation)
GLUT_MIDDLE_BUTTON (1)  → Secondary (could pan camera)
GLUT_RIGHT_BUTTON (2)   → Context menu (unused here)

This game only responds to left button
```

**Step 2a: Drag Initiation (Button Pressed)**
```cpp
if (state == GLUT_DOWN) {
    isDragging = true;
    lastMouseX = x;
    lastMouseY = y;
```

**Drag Start Sequence:**
```
Event: Left mouse button pressed at screen (450, 300)

Execution:
  isDragging = true     (enter drag mode)
  lastMouseX = 450      (baseline X position)
  lastMouseY = 300      (baseline Y position)
  
Result: Ready to calculate delta on next mouse motion
```

**Why Store Baseline:**
- Every pixel moved calculates delta from baseline
- Delta (currentX - lastMouseX) = mouse displacement
- Used to convert pixels to camera rotation angles

**Example Sequence:**
```
t=0ms:   Mouse button down at (400, 300)
         isDragging=true, lastMouseX=400, lastMouseY=300
         
t=16ms:  Mouse motion event at (450, 300)
         In mouseMotion callback:
           deltaX = 450 - 400 = 50 pixels right
           camera rotates ~0.5 radians right
           lastMouseX = 450 (update baseline)
           
t=32ms:  Mouse motion at (470, 350)
         In mouseMotion:
           deltaX = 470 - 450 = 20 pixels
           deltaY = 350 - 300 = 50 pixels down
           camera rotates 20px right + 50px down
           lastMouseX = 470, lastMouseY = 350
           
t=50ms:  Mouse button up
         isDragging=false (exit drag mode)
         Camera stops responding to mouse movement
```

**Step 2b: Drag Termination (Button Released)**
```cpp
} else if (state == GLUT_UP) {
    isDragging = false;
```

**Drag End:**
```
When left button released:
  isDragging = false (exit drag mode)
  
Result: mouseMotion() will skip processing (isDragging check fails)
        Camera no longer rotates with mouse movement
```

### Mouse Motion Callback

```cpp
void mouseMotion(int x, int y) {
    if (isDragging) {
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;
        lastMouseX = x;
        lastMouseY = y;
        camera->processMouseDrag(deltaX, deltaY);
    }
}
```

**Detailed Explanation - Continuous Mouse Drag Processing:**

**Algorithm Type**: Differential tracking with state-gated processing

**Purpose**: Convert real-time mouse movement to camera rotation

**Step 1: Guard Check**
```cpp
if (isDragging) {
```

**State Gate:**
```
If isDragging = false:
  Function returns immediately (even if mouse moves)
  Camera unresponsive
  
If isDragging = true:
  Process mouse movement into camera rotation
  
Use Case: Player can move mouse freely without rotating camera
```

**Step 2: Calculate Delta Movement**
```cpp
int deltaX = x - lastMouseX;
int deltaY = y - lastMouseY;
```

**Example Timeline (User Drags Mouse Right):**
```
Frame 1: lastMouseX=400, current x=450
         deltaX = 450 - 400 = +50 pixels (dragged right)
         
Frame 2: lastMouseX=450, current x=470
         deltaX = 470 - 450 = +20 pixels (slower drag right)
         
Frame 3: lastMouseX=470, current x=460
         deltaX = 460 - 470 = -10 pixels (dragged LEFT!)
         
Frame 4: lastMouseX=460, current x=460
         deltaX = 0 (no movement)
```

**Coordinate Spaces:**
```
Screen coordinates: (0,0) at top-left
  X: 0 (left) → 800 (right)
  Y: 0 (top) → 600 (bottom)
  
Delta space: difference in pixels
  +deltaX: right movement
  -deltaX: left movement
  +deltaY: down movement
  -deltaY: up movement
```

**Step 3: Update Baseline for Next Frame**
```cpp
lastMouseX = x;
lastMouseY = y;
```

**Continuous Tracking:**
- After calculating delta, move baseline to current position
- Next frame: current position becomes previous position
- Creates seamless tracking across all frames

**Step 4: Pass to Camera**
```cpp
camera->processMouseDrag(deltaX, deltaY);
```

**Camera Processing:**
```
Inside Camera::processMouseDrag(deltaX, deltaY):
  targetYaw += deltaX * mouseSensitivity;
  targetPitch += deltaY * mouseSensitivity;
  
  With mouseSensitivity = 0.01:
    +50 pixels → +0.5 radians yaw (~29 degrees)
    +20 pixels → +0.2 radians pitch (~11 degrees)
  
  Camera then lerps smoothly to new target over ~0.3 seconds
  (exponential lerp at 7.0x speed from earlier expansion)
```

**Latency Analysis:**
```
User moves mouse:
  t=0ms:     Physical mouse moved
  t=1ms:     OS detects motion
  t=5ms:     GLUT polls mouse position
  t=6ms:     mouseMotion() callback invoked
  t=7ms:     camera->processMouseDrag() updates target
  t=8ms:     Next idle() call
  t=8.33ms:  camera update lerps toward target
  t=16.67ms: display() renders at lerped position
  
Total: 8-16ms (acceptable for game, ~1 frame latency)
```

**GLUT Callback Registration:**
```cpp
glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    // NOT glutPassiveMotionFunc (button required, not passive)
```

**Key GLUT Distinction:**
```
glutMotionFunc(mouseMotion):
  Called when MOUSE BUTTON HELD + MOUSE MOVES
  Used for: drag operations, camera control
  
glutPassiveMotionFunc():
  Called when MOUSE MOVES (NO buttons held)
  Used for: hover effects, tooltips
  Not used in this game
```
- On mouse down: sets `isDragging = true` and saves position
- On mouse up: sets `isDragging = false` to stop camera rotation

### Mouse Motion Handling

```cpp
void mouseMotion(int x, int y) {
    if (isDragging) {
        float deltaX = (float)(x - lastMouseX);
        float deltaY = (float)(y - lastMouseY);
        
        lastMouseX = x;
        lastMouseY = y;
        
        game->onMouseDrag(deltaX, deltaY);
    }
}
```

**Explanation:**
- Called whenever mouse moves while a button is held
- Only processes if left button is being dragged
- Calculates change in position (delta) between current and last position
- Passes delta to camera for smooth rotation/orbit control

### Window Reshape Handler

```cpp
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    game->onResize(w, h);
}
```

**Explanation:**
- Called when window is resized by user
- Prevents division by zero if height becomes 0
- Updates OpenGL viewport to match new window dimensions
- Notifies game of new dimensions for UI scaling

### Main Function

```cpp
int main(int argc, char** argv) {
    SaveManager::initialize(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Crazy Hopper");

    glewInit();
    game = new Game(800, 600);
    game->initialize();

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}
```

**Detailed Explanation - Application Initialization Pipeline:**

**Step 1: Save Manager Initialization**
```
SaveManager::initialize(argv[0])
```
- **Input**: `argv[0]` is the full path to the executable binary (provided by OS)
- **Algorithm**: Extracts directory path from executable path using `std::filesystem::path`
  - `fs::path exePath(argv[0])` - Convert string to path object
  - `exePath.parent_path()` - Get directory containing executable
  - Construct `savedata.dat` filename in that directory
- **Use Case**: Ensures save file is placed next to executable, making game portable
- **Why This Matters**: When user runs `./CS302N_Game`, save file goes to same directory, not random system locations

**Step 2: GLUT Window Context Setup**
```
glutInit(&argc, argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE)
glutInitWindowSize(800, 600)
glutCreateWindow("Crazy Hopper")
```
- **Algorithm**: GLUT sequentially initializes windowing system
- **Display Mode Flags** (bitwise OR combined):
  - `GLUT_DOUBLE`: Double buffering technique - maintains front and back buffers. Rendering happens to back buffer while front buffer displays to screen, then swap occurs atomically. Eliminates flickering/tearing.
  - `GLUT_RGB`: 24-bit color (8-bit each R, G, B channels) vs indexed color mode
  - `GLUT_DEPTH`: Allocates 24/32-bit depth buffer for Z-testing. Prevents drawing distant objects over closer ones. Each pixel stores Z-value; only updates if new Z is closer.
  - `GLUT_MULTISAMPLE`: Enables MSAA (Multi-Sample Anti-Aliasing). Renders at higher internal resolution, samples 4x per pixel, averages results for smooth edges
- **Window Size**: 800×600 pixel resolution (4:3 aspect ratio, traditional)
- **Window Title**: "Crazy Hopper" appears in window frame

**Step 3: OpenGL Extension Loader Initialization**
```
glewInit()
```
- **Purpose**: GLEW = OpenGL Extension Wrangler Library
- **What it does**: At runtime, queries graphics driver for available OpenGL functions. Modern OpenGL requires loading function pointers at runtime (unlike older fixed-function pipeline).
- **Without this**: All `glDrawArrays()`, `glCreateShader()` etc. would be NULL pointers - segfault on call
- **Must be called AFTER**: Window context created (glewInit needs valid OpenGL context)

**Step 4: Game Object Creation & Initialization**
```
game = new Game(800, 600)
game->initialize()
```
- **Constructor**: Allocates Game object, passes window dimensions, initializes member variables:
  - Sets state to `GAME_STATE_MAIN_MENU`
  - Loads persistent data (coins, high score) from save file via SaveManager
  - Seeds random number generator: `srand(time(nullptr))`
- **initialize()**: Sets up renderer:
  - Compiles shaders from disk
  - Loads textures into VRAM
  - Initializes OpenGL state (lighting, blend modes, etc.)
  - Calls `resetGame()` to populate initial lanes

**Step 5: Event Callback Registration**
```
glutDisplayFunc(display)        // Called when window needs redraw
glutIdleFunc(idle)              // Called every frame (main loop)
glutKeyboardFunc(keyboard)      // Called on keyboard input
glutSpecialFunc(specialKey)     // Called on arrow keys, function keys
glutReshapeFunc(reshape)        // Called when window resized
glutMouseFunc(mouseButton)      // Called on mouse click
glutMotionFunc(mouseMotion)     // Called on mouse movement with button held
```
- **Algorithm**: GLUT maintains callback function pointers internally
- **When registered**: Each event type gets function pointer stored
- **Event Loop**: `glutMainLoop()` (below) blocks and enters event dispatch:
  1. Queries OS for pending events (keyboard, mouse, window resize)
  2. Calls appropriate registered callback for each event
  3. Calls idle callback if no events pending (continuous update)
  4. Returns to step 1

**Step 6: Main Event Loop Entry**
```
glutMainLoop()
return 0
```
- **Control Flow**: Once entered, `glutMainLoop()` blocks indefinitely. Never returns until:
  - User closes window (glutLeaveMainLoop() called, or ESC key triggers `exit(0)`)
  - Program terminates
- **Execution Model**: GLUT takes over thread; all game logic runs via callbacks
- **Return Statement**: Technically unreachable unless main loop exits normally
- **Process Lifecycle**: Program stays alive as long as glutMainLoop() runs

**Complete Initialization Flow:**
```
Executable starts
    ↓
main() called with argv[0]
    ↓
SaveManager finds save directory
    ↓
GLUT creates 800×600 window with OpenGL context
    ↓
GLEW loads all OpenGL function pointers
    ↓
Game object created, loads data from disk
    ↓
Renderer initializes (shaders, textures, GL state)
    ↓
Callbacks registered with GLUT
    ↓
glutMainLoop() blocks and starts event dispatch
    ↓
[Events occur - callbacks execute continuously]
    ↓
[Stays in loop until exit triggered]
```

---

## 2. types.h - Game Configuration & Enumerations

### Game State Enum

```cpp
enum GameState { 
    GAME_STATE_MAIN_MENU, 
    GAME_STATE_CHARACTER_SELECT, 
    GAME_STATE_START_SCREEN, 
    GAME_STATE_PLAYING, 
    GAME_STATE_GAME_OVER 
};
```

**Explanation:** Defines all possible game states:
- `MAIN_MENU`: Title screen with high score display
- `CHARACTER_SELECT`: Menu for choosing player character (Chicken, Frog, Dino, Cat, Dog)
- `START_SCREEN`: Pre-game screen before gameplay
- `PLAYING`: Active gameplay
- `GAME_OVER`: Death screen with restart option

### Lane Types

```cpp
enum LaneType  { LANE_GRASS, LANE_ROAD, LANE_RAIL, LANE_RIVER, LANE_LILYPAD };
```

**Explanation:** Types of terrain the player crosses:
- `GRASS`: Safe zones with trees/rocks for decoration
- `ROAD`: Lanes with cars moving horizontally
- `RAIL`: Train tracks with oncoming trains
- `RIVER`: Moving logs the player must navigate
- `LILYPAD`: Stationary lilypads in water (alternative to logs)

### Obstacle Types

```cpp
enum ObstacleType { OBSTACLE_CAR, OBSTACLE_TRAIN, OBSTACLE_LOG, OBSTACLE_LILYPAD };
```

**Explanation:** Different hazards that can kill player or provide passage:
- `CAR`: Various vehicle types (small car, big car, truck)
- `TRAIN`: Long multi-segment train
- `LOG`: Floating log player stands on (must manage X position or fall in water)
- `LILYPAD`: Stationary lily pads for water crossing

### Vehicle Variants

```cpp
enum VehicleVariant { VEHICLE_SMALL_CAR, VEHICLE_BIG_CAR, VEHICLE_TRUCK };
```

**Explanation:** Different sizes of cars for visual variety, each with different hitbox sizes

### Configuration Namespace - World Parameters

```cpp
namespace Config {
    constexpr float CELL_SIZE = 1.0f;
    constexpr int   INITIAL_SAFE_ZONE_LENGTH = 15;
    constexpr float HITBOX_PADDING = 0.8f;
```

**Explanation:**
- `CELL_SIZE`: Grid unit size (1.0 = one cell square)
- `INITIAL_SAFE_ZONE_LENGTH`: 15 grass lanes at start before hazards spawn
- `HITBOX_PADDING`: 0.8x reduction of collision boxes for forgiving gameplay (easier to avoid hazards)

### Boundary Configuration

```cpp
    constexpr float BOUNDARY_X = 13.5f;
    constexpr float MOUNTAIN_WALL_X = 14.5f;
    constexpr int   MOUNTAIN_COLS = 8;
    constexpr float BOUNDARY_BACK_Z = 2.0f;
```

**Explanation:**
- `BOUNDARY_X`: ±13.5 is where player movement stops (width of playable area)
- `MOUNTAIN_WALL_X`: ±14.5 where visual mountain walls are drawn
- `MOUNTAIN_COLS`: Mountain extends 8 columns beyond inner wall for depth
- `BOUNDARY_BACK_Z`: Positive Z limit prevents moving backward past start

### Lane Generation Ranges

```cpp
    constexpr int MIN_GRASS_WIDTH = 1;  constexpr int MAX_GRASS_WIDTH = 3;
    constexpr int MIN_ROAD_WIDTH = 1;   constexpr int MAX_ROAD_WIDTH = 3;
    constexpr int MIN_RAIL_WIDTH = 1;   constexpr int MAX_RAIL_WIDTH = 2;
    constexpr int MIN_RIVER_WIDTH = 1;  constexpr int MAX_RIVER_WIDTH = 3;
```

**Explanation:** Random ranges for lane block widths. Lane blocks randomly vary in width to keep gameplay unpredictable. For example, a road block might be 1-3 lanes wide.

### Rendering & Performance

```cpp
    constexpr int   TARGET_FPS = 120;
    constexpr float FRAME_TIME = 1.0f / TARGET_FPS;
    constexpr float GAME_SPEED_MULTIPLIER = 2.6f;
```

**Explanation:**
- Target 120 FPS for smooth 3D rendering
- `FRAME_TIME = 0.00833...` seconds between updates
- `GAME_SPEED_MULTIPLIER = 2.6x` speeds up internal game logic compared to real time

### Camera Configuration

```cpp
    constexpr float CAMERA_AUTO_SCROLL_SPEED = 0.0f;
    constexpr float CAMERA_BACKWARD_DEATH_DISTANCE = 5.0f;
    constexpr float LANE_GENERATION_BUFFER_AHEAD = 25.0f;
    constexpr float LANE_CLEANUP_BUFFER_BEHIND = 12.0f;
    constexpr float CAMERA_SMOOTH_SPEED_XY = 1.0f;
    constexpr float CAMERA_SMOOTH_SPEED_Z = 1.0f;
```

**Explanation:**
- No automatic forward scrolling (player controls forward/backward)
- Player dies if they move 5+ units backward past starting camera position
- Lanes generated 25 units ahead of player to ensure smooth streaming
- Lanes cleaned up 12 units behind player to save memory
- Smooth camera movement interpolation speeds (1.0 = moderate smoothing)

### Vehicle Movement

```cpp
    constexpr float CAR_SPEED_MIN = 2.0f;
    constexpr float CAR_SPEED_MAX = 4.0f;
    constexpr float TRAIN_SPEED = 25.0f;
```

**Explanation:**
- Cars move between 2-4 units/second randomly
- Trains move much faster at 25 units/second

### Log Physics

```cpp
    constexpr float LOG_SPEED_MIN = 0.75f;
    constexpr float LOG_SPEED_MAX = 1.5f;
    constexpr int   LOG_COUNT_MIN = 2;
    constexpr int   LOG_COUNT_MAX = 3;
    constexpr float LOG_SPACING = 4.0f;
    constexpr float LOG_WIDTH = 2.8f;
    constexpr float LOG_DEPTH = 0.75f;
    constexpr float LOG_HEIGHT = 0.28f;
    constexpr float LOG_Y = 0.14f;
    constexpr int LOG_SETS = 2;
    constexpr float LOG_SET_GAP = 15.0f;
```

**Explanation:**
- Logs float at 0.75-1.5 units/second
- Spawn 2-3 logs per set with 4.0 units spacing
- Logs are 2.8 wide × 0.28 tall × 0.75 deep
- Two separate "sets" of logs per river lane with 15 units between sets

### Log Sinking

```cpp
    constexpr float LOG_SINK_AMOUNT = 0.07f;
    constexpr float LOG_SINK_SPEED = 6.0f;
```

**Explanation:**
- When player stands on log, it sinks 0.07 units
- Sinking lerps smoothly at 6.0 speed factor

### Fast-Stream Physics

```cpp
    constexpr float LOG_STREAM_TRIGGER_X = 12.0f;
    constexpr float LOG_STREAM_ACCEL = 6.0f;
    constexpr float LOG_STREAM_MAX_SPEED = 22.0f;
    constexpr float LOG_STREAM_EXIT_X = 20.0f;
```

**Explanation:** When log reaches foam boundary (±12 units), it enters "fast-stream" mode:
- Exponentially accelerates at 6x rate
- Speed capped at 22 units/second max
- Doesn't wrap around screen until reaching ±20 units (exits completely)

### Lilypad Spawning

```cpp
    constexpr float LILYPAD_SIZE = 0.9f;
    constexpr float LILYPAD_HEIGHT = 0.15f;
    constexpr float LILYPAD_Y = -0.05f;
    constexpr float LILYPAD_GAP_MIN = 1.2f;
    constexpr float LILYPAD_GAP_MAX = 10.0f;
```

**Explanation:**
- Lilypads are 0.9×0.15×0.9 units
- Spawn with random gaps between 1.2-10.0 units
- One guaranteed lilypad always at safe path column for guaranteed crossing

### Day/Night Cycle

```cpp
    constexpr float TIME_SPEED = 0.01f;
    constexpr float TRANSITION_SMOOTHNESS = 4.0f;
    constexpr float TRANSITION_ZONE_WIDTH = 0.20f;
    constexpr float SUN_LIGHT_INTENSITY = 1.2f;
    constexpr float MOON_LIGHT_INTENSITY = 0.5f;
    constexpr glm::vec3 LIGHT_DIRECTION_VECTOR = glm::vec3(1.0f, 1.0f, 0.0f);
```

**Explanation:**
- Day/night cycle completes every 100 seconds (1.0 / 0.01)
- Smooth 4-second transition between day and night
- Sun moves left-to-right across sky (1.0, 1.0, 0.0)
- Sun at 1.2x brightness, moon at 0.5x

### Shadow Rendering

```cpp
    constexpr float SHADOW_Y_OFFSET = 0.02f;
    constexpr float SHADOW_Z_OFFSET = -0.1f;
    constexpr float SHADOW_OPACITY = 0.4f;
    constexpr float SHADOW_MAX_LENGTH = 3.0f;
    constexpr float SHADOW_MIN_LENGTH = 0.5f;
    constexpr float SHADOW_FADE_START_ANGLE = 0.524f;
    constexpr float SHADOW_FADE_DURATION = 0.5f;
```

**Explanation:**
- Shadows slightly above ground (0.02) to prevent z-fighting
- Shadow opacity 40% for visibility without dominating
- Shadows fade when sun near horizon (0.524 rad ≈ 30°)
- Lane-specific shadow heights for visual consistency

### Utility Function

```cpp
inline float randomRange(float min, float max) {
    return min + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min);
}
```

**Explanation:** Generates random float between min and max. Used throughout for random obstacle speeds, lane widths, etc.

---

## 3. save_data.h & save_data.cpp - Persistent Data

### Header

```cpp
class SaveManager {
public:
    static void initialize(const char* executablePath);
    static bool loadData(uint64_t& outCoins, uint64_t& outHighScore);
    static void saveData(uint64_t totalCoins, uint64_t highScore);
};
```

**Explanation:** Static class for loading/saving game progress without instantiation.

### Implementation

```cpp
static std::string g_saveFilePath;

void SaveManager::initialize(const char* executablePath) {
    fs::path exePath(executablePath);
    fs::path saveDir = exePath.parent_path();
    g_saveFilePath = (saveDir / "savedata.dat").string();
}
```

**Explanation:**
- Takes executable path (argv[0] from main)
- Extracts directory path using `std::filesystem`
- Creates `savedata.dat` in same directory as executable

### Loading Data

```cpp
bool SaveManager::loadData(uint64_t& outCoins, uint64_t& outHighScore) {
    std::ifstream file(g_saveFilePath, std::ios::binary);
    if (!file.is_open()) {
        return false; // First time playing
    }

    file.read(reinterpret_cast<char*>(&outCoins), sizeof(uint64_t));
    file.read(reinterpret_cast<char*>(&outHighScore), sizeof(uint64_t));
    file.close();
    return true;
}
```

**Explanation:**
- Opens save file in binary mode (128 bits total = 8 bytes per value × 2)
- Returns false if file doesn't exist (first-time player)
- Reads two 64-bit unsigned integers directly from binary file
- First 64 bits = total coins collected, next 64 bits = high score

### Saving Data

```cpp
void SaveManager::saveData(uint64_t totalCoins, uint64_t highScore) {
    std::ofstream file(g_saveFilePath, std::ios::binary | std::ios::trunc);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&totalCoins), sizeof(uint64_t));
        file.write(reinterpret_cast<const char*>(&highScore), sizeof(uint64_t));
        file.close();
    }
}
```

**Explanation:**
- Opens file in binary write mode with truncate (overwrites existing data)
- Writes coins and high score as raw 64-bit values
- Trunc ensures clean overwrite without leftover data

---

# GAME ENGINE FILES

## 4. game.h - Main Game Class Header

```cpp
class Game {
private:
    int windowWidth, windowHeight;
    GameState state;
    Renderer renderer;
    Camera camera;
    Chicken player;
    PreGameManager preGameManager;
    std::vector<Lane> lanes;
    float currentGenerationZ;
    float cameraTrackZ;
    int score;
    int highScore; 
    float startZ;
    int coinScore = 0;
    int totalCoins = 0;
```

**Explanation:**
- Core Game class contains all game data
- `state`: Current game state (menu, playing, etc.)
- `renderer`: Graphics engine
- `camera`: View control
- `player`: Chicken character
- `lanes`: Vector of all active lanes (infinite stream)
- `currentGenerationZ`: Z coordinate for next lane generation
- `score/highScore`: Player progression tracking
- `coinScore`: Coins collected this run
- `totalCoins`: Total coins across all games (saved)

### Death Tracking

```cpp
    glm::vec3 deathPosition;
    bool hasWaterDeath;
    bool hasStreamDeath;
```

**Explanation:**
- `deathPosition`: Where player died for camera zoom-in on death
- `hasWaterDeath`: Player fell in water (used for particle effects)
- `hasStreamDeath`: Player swept away by fast-stream log

### Camera & UI State

```cpp
    glm::vec3 smoothedCameraTarget;
    int eggClicks = 0;
    int lastClickTime = 0;
    int lastFrameTime = 0;
    bool nightMode = false;
    int selectedCharacterIndex = 0;
    float currentGameTime = 30.0f;
    float sunAngle = 0.0f;
```

**Explanation:**
- `smoothedCameraTarget`: Lerped camera position for smooth following
- `eggClicks`: Counter for egg clicks on main menu
- `nightMode`: Day/night cycle flag
- `selectedCharacterIndex`: Which character selected (0-4)
- `currentGameTime/sunAngle`: Day/night cycle tracking

---

## 5. game.cpp - Main Game Implementation

### Constructor

```cpp
Game::Game(int width, int height)
    : windowWidth(width), windowHeight(height),
      state(GAME_STATE_MAIN_MENU), 
      preGameManager(width, height),
      currentGenerationZ(5.0f * Config::CELL_SIZE),
      cameraTrackZ(0.0f), score(0), highScore(0), startZ(0.0f), coinScore(0), totalCoins(0),
      deathPosition(0.0f), hasWaterDeath(false), hasStreamDeath(false),
      lastClickTime(0)
{
    srand(static_cast<unsigned>(time(nullptr)));
    
    uint64_t loadedCoins = 0;
    uint64_t loadedScore = 0;
    if (SaveManager::loadData(loadedCoins, loadedScore)) {
        totalCoins = loadedCoins;
        highScore = loadedScore;
    } else {
        totalCoins = 0;
        highScore = 0;
    }
}
```

**Explanation:**
- Initializes all game state variables
- Seeds random number generator with current time
- Loads persistent data (coins and high score) from save file
- If no save file exists, sta
```

**Detailed Explanation - Per-Frame Collision Detection System:**

**Algorithm Type**: Multi-layer spatial partitioning with early exits

**Purpose**: Check all hazards each frame and trigger appropriate death/movement states

**Inputs:**
- `playerPos`: Position during jump animation (mid-air during jump, ground when landed)
- `playerSize`: Collision bounding box (0.8×0.8×0.8 units, padded to 0.64)
- `playerBasePos`: Stable ground position (what we save for next spawn)
- `deltaTime`: Frame time for physics (not used in collision, only death animations)

**Step 1: Find Current Lane**
```cpp
for (auto &lane : lanes) {
    if (std::abs(lane.getZPosition() - playerPos.z) < Config::CELL_SIZE / 2.0f) {
        currentLane = &lane;
        break;
    }
}
if (!currentLane) return;
```

**Lane Detection Algorithm:**

```
Lane Grid (vertical strips, each 1 unit tall):
  Z Position:  0    -1    -2    -3    -4   ...
  Lane:       [0]  [1]  [2]  [3]  [4]  ...
  Type:      GRS  GRS  GRS  ROAD ROAD ...
  Obstacles:  -    -    -   [cars][cars] ...

Player Position: Z = -3.2

Check each lane:
  |0 - (-3.2)| = 3.2 > 0.5 (threshold) ✗
  |-1 - (-3.2)| = 2.2 > 0.5 ✗
  |-2 - (-3.2)| = 1.2 > 0.5 ✗
  |-3 - (-3.2)| = 0.2 < 0.5 ✓ MATCH!
  
  currentLane points to lane 3 (ROAD lane with cars)
  break (exit early, don't check remaining lanes)
```

**Why ±0.5 Tolerance:**
```
Config::CELL_SIZE = 1.0 (lanes are 1 unit apart)
Tolerance = 1.0 / 2 = 0.5

Rationale:
  Player at Z = -3.0 (exactly on lane edge):
    Could be ending lane 2 or starting lane 3
    Tolerance ±0.5 puts them in lane 3
  
  Player at Z = -3.2 (during jump toward lane 3):
    Distance from lane 3 = 0.2 < 0.5
    Confirmed: in lane 3 landing zone
  
  Player at Z = -2.8 (bounced back):
    Distance from lane 3 = 0.2 < 0.5
    Still consider lane 3 (forgiveness)
```

**Early Exit Optimization:**
```cpp
if (!currentLane) return;
```

Bailed if:
- Player somehow off all lanes (shouldn't happen)
- No valid lane found (edge case)
- Prevents null pointer crash in subsequent collision checksrts with 0

### Reset Game

```cpp
void Game::resetGame()
{
    eggClicks = 0;
    coinScore = 0;
    score = 0;
    hasWaterDeath = false;
    hasStreamDeath = false;
    deathPosition = glm::vec3(0.0f);
    currentGameTime = 30.0f;  

    player.reset();
    camera.resetToDefault();

    lanes.clear();
    currentGenerationZ = 5.0f * Config::CELL_SIZE;
    safePathColumn = 0;

    for (int i = 0; i < Config::INITIAL_SAFE_ZONE_LENGTH; i++)
    {
        lanes.push_back(Lane(currentGenerationZ, LANE_GRASS, safePathColumn));
        currentGenerationZ -= Config::CELL_SIZE;
    }
    for (int i = 0; i < 5; i++)
        generateLaneBlock();

    cameraTrackZ = player.getPosition().z;
    startZ = player.getPosition().z;

    state = GAME_STATE_MAIN_MENU;
}
```

**Explanation:**
- Resets per-run statistics (score, coins, death state)
- Clears all lanes and rebuilds initial setup
- Creates 15 safe grass lanes at start
- Pre-generates 5 lane blocks ahead
- Resets camera and player position
- Returns to main menu state

### Lane Generation

```cpp
void Game::generateLaneBlock()
{
    int r = rand() % 100;
    LaneType nextType;
    int blockWidth = 1;

    if (r < 30) {
        nextType = LANE_GRASS;
        blockWidth = Config::MIN_GRASS_WIDTH + rand() % (Config::MAX_GRASS_WIDTH - Config::MIN_GRASS_WIDTH + 1);
    }
    else if (r < 60) {
        nextType = LANE_ROAD;
        blockWidth = Config::MIN_ROAD_WIDTH + rand() % (Config::MAX_ROAD_WIDTH - Config::MIN_ROAD_WIDTH + 1);
    }
    else if (r < 80) {
        nextType = LANE_RIVER;
        blockWidth = Config::MIN_RIVER_WIDTH + rand() % (Config::MAX_RIVER_WIDTH - Config::MIN_RIVER_WIDTH + 1);
    }
    else {
        nextType = LANE_RAIL;
        blockWidth = Config::MIN_RAIL_WIDTH + rand() % (Config::MAX_RAIL_WIDTH - Config::MIN_RAIL_WIDTH + 1);
    }
```

**Detailed Explanation - Procedural Lane Generation Algorithm:**

**Algorithm Type**: Weighted random generation with constraints

**Purpose**: Infinitely generate playable lane patterns on-the-fly

**Inputs:**
- Current state: `lanes` vector, `currentGenerationZ`, `safePathColumn`
- Configuration: All the Config:: parameters

**Step 1: Type Selection (Weighted Random)**

```cpp
int r = rand() % 100;  // Random 0-99
```

- Generates single random integer 0-99
- Used to select lane type with weighted probability:

```
Range    Type         Probability
0-29:    GRASS        30%
30-59:   ROAD         30%
60-79:   RIVER        20%
80-99:   RAIL         20%
```

**Why These Probabilities:**
- Grass: Safe zones, frequent (let player breathe)
- Road: Moderate threat, frequent (main hazard)
- River: More difficult, less frequent (pacing variation)
- Rail: Very difficult (train is fast), least frequent (save for challenge)

**Example**: `rand()` returns 42
- 42 < 30? No
- 42 < 60? Yes → Select LANE_ROAD

**Step 2: Block Width Randomization**

```cpp
blockWidth = Config::MIN_GRASS_WIDTH + rand() % (Config::MAX_GRASS_WIDTH - Config::MIN_GRASS_WIDTH + 1);
```

**Formula Breakdown:**
- Config::MIN_GRASS_WIDTH = 1
- Config::MAX_GRASS_WIDTH = 3
- Range expression: `rand() % (3 - 1 + 1)` = `rand() % 3`
- Results in 0, 1, or 2
- Adding MIN: 1 + {0,1,2} = {1,2,3}
- **Result**: blockWidth randomly 1-3 lanes wide

**Why Vary Block Width:**
- Prevents monotonous repeating pattern
- 1-lane block: tight, challenging
- 2-lane block: medium difficulty
- 3-lane block: more forgiving (multiple safe paths)

**Different Ranges per Type:**
```
Grass:  1-3 lanes (all values)
Road:   1-3 lanes (all values)
River:  1-3 lanes (all values)
Rail:   1-2 lanes (train blocks narrower, makes them harder)
```

### Buffer Zone Between Hazard Types

```cpp
    if (!lanes.empty()) {
        LaneType prevBlockType = lanes.back().getType();
        bool prevIsTunnel = (prevBlockType == LANE_ROAD || prevBlockType == LANE_RAIL);
        bool nextIsTunnel = (nextType == LANE_ROAD || nextType == LANE_RAIL);

        if (prevIsTunnel && nextIsTunnel && prevBlockType != nextType) {
            int bufWidth = Config::MIN_GRASS_WIDTH + rand() % (Config::MAX_GRASS_WIDTH - Config::MIN_GRASS_WIDTH + 1);
            for (int g = 0; g < bufWidth; g++) {
                int shift = rand() % 3 - 1;
                safePathColumn = std::max(-5, std::min(5, safePathColumn + shift));
                lanes.push_back(Lane(currentGenerationZ, LANE_GRASS, safePathColumn));
                currentGenerationZ -= Config::CELL_SIZE;
            }
        }
    }
```

**Step 3: Buffer Insertion (Difficulty Smoothing)**

**Logic Breakdown:**

**Identify Hazard Types:**
```
prevIsTunnel = (ROAD or RAIL)?  → hazard that requires avoidance
nextIsTunnel = (ROAD or RAIL)?  → next will be hazard
```

**Condition to Insert Buffer:**
```
prevIsTunnel && nextIsTunnel && prevBlockType != nextType
↓
Previous is hazard AND Next is hazard AND Types differ
```

**Example Scenarios:**

```
Scenario 1: GRASS → ROAD → RIVER
  prev=GRASS, next=ROAD
  prevIsTunnel=false, nextIsTunnel=true
  Condition: false && true → NO buffer (safe to hazard is OK)

Scenario 2: ROAD → RAIL (transition between hazards)
  prev=ROAD, next=RAIL
  prevIsTunnel=true, nextIsTunnel=true, types different
  Condition: true && true && true → INSERT BUFFER ✓
  [1-3 grass lanes inserted as break]
  Result: ROAD lanes → [BUFFER GRASS] → RAIL lanes

Scenario 3: ROAD → ROAD (same hazard continues)
  prev=ROAD, next=ROAD
  prevIsTunnel=true, nextIsTunnel=true, types SAME
  Condition: true && true && false → NO buffer
  Result: ROAD lanes continue naturally
```

**Why Insert Buffer:**
- Prevents impossible patterns like car→train with no time to adjust
- Gives player reaction time
- Breaks up difficulty monotony
- Smooth pacing: HAZARD → REST → HAZARD

### Safe Path Tracking

```cpp
    for (int i = 0; i < blockWidth; i++) {
        int shift = rand() % 3 - 1;
        safePathColumn += shift;
        if (safePathColumn < -5) safePathColumn = -5;
        if (safePathColumn > 5) safePathColumn = 5;

        LaneType actualType = nextType;
        if (nextType == LANE_RIVER) {
            bool prevIsLilypad = (!lanes.empty() && lanes.back().getType() == LANE_LILYPAD);
            if (!prevIsLilypad && (rand() % 100 < 40))
                actualType = LANE_LILYPAD;
        }

        lanes.push_back(Lane(currentGenerationZ, actualType, safePathColumn));
        currentGenerationZ -= Config::CELL_SIZE;
    }
}
```

**Step 4: Generate Lane Block**

**Loop Iteration:**
- For each lane in the block (1-3 iterations typically)

**Safe Path Movement:**
```cpp
int shift = rand() % 3 - 1;  // -1, 0, or +1
safePathColumn += shift;     // Move safe path left/right
```
- Creates serpentine path (wiggling left-right)
- Prevents straight corridors
- Makes navigation require active input

**Visual Example - Safe Path Over 5 Lanes:**
```
Lane 1: safePathColumn = 0  [Grass at column 0, obstacles elsewhere]
Lane 2: shift=-1 → column=-1 [Safe path moved left]
Lane 3: shift=+1 → column=0  [Safe path moved right]
Lane 4: shift=+1 → column=+1 [Safe path moved right]
Lane 5: shift=0  → column=+1 [Safe path stays right]

Visual from above (X is safe, O is hazard):
Lane 1: O O X O O
Lane 2: O X O O O
Lane 3: O O X O O
Lane 4: O O O X O
Lane 5: O O O X O

Player must navigate: right-left-right pattern
```

**Boundary Clamping:**
```cpp
if (safePathColumn < -5) safePathColumn = -5;
if (safePathColumn > 5) safePathColumn = 5;
```
- Prevents safe path from wandering off edge
- Keeps it within playable zone (-5 to +5 = 11 columns × 1 unit)

**River Lane Variant (40% Chance):**
```cpp
if (nextType == LANE_RIVER)
{
    bool prevIsLilypad = (!lanes.empty() && lanes.back().getType() == LANE_LILYPAD);
    if (!prevIsLilypad && (rand() % 100 < 40))
        actualType = LANE_LILYPAD;
}
```
- When generating river lane, 40% chance becomes LILYPAD instead
- But NOT if previous lane was already lilypad (prevents lilypad spam)
- Creates variation: logs, lilypads, logs, logs, lilypads pattern

**Lane Creation:**
```cpp
lanes.push_back(Lane(currentGenerationZ, actualType, safePathColumn));
currentGenerationZ -= Config::CELL_SIZE;
```
- Constructs Lane object (triggers all its generation - obstacles, coins, decorations)
- Adds to lanes vector
- Moves generation pointer backward by 1 grid cell

**Complete Generation Flow Example:**

```
Initial state: lanes = [15 GRASS lanes]

Call generateLaneBlock():
  rand() % 100 = 45 → ROAD lane selected (30-59 range)
  blockWidth = 1 + (rand() % 3) = 2 lanes wide
  
  prevType = GRASS (last lane in vector)
  prevIsTunnel = false
  nextIsTunnel = true (ROAD is hazard)
  → NO buffer inserted (safe→hazard transition OK)
  
  Generate 2 ROAD lanes:
    Lane 1: safePathColumn=0, shift=-1 → column=-1
            Push(z=4, ROAD, col=-1)
            currentGenerationZ = 3
    Lane 2: safePathColumn=-1, shift=+1 → column=0
            Push(z=3, ROAD, col=0)
            currentGenerationZ = 2

Result: lanes = [15 GRASS] + [2 ROAD] = 17 lanes
Next call will generate from z=2 onward
```

**Overall Result After Many Calls:**

```
Z= 0: GRASS (safe start)
Z=-1: GRASS
...
Z=-14: GRASS
Z=-15: ROAD (cars)
Z=-16: ROAD
Z=-17: ROAD
Z=-18: GRASS (buffer)
Z=-19: GRASS
Z=-20: RAIL (train) - separated from ROAD
Z=-21: RAIL
Z=-22: RIVER (logs)
Z=-23: RIVER
Z=-24: LILYPAD (river variant)
Z=-25: GRASS
...infinite pattern continues
```

**Benefits of This Algorithm:**
1. **Infinite Level**: Can generate forever without repeating
2. **Paced Difficulty**: Varies threat levels smoothly
3. **Playable**: Buffers ensure transitions aren't impossible
4. **Varied**: Block widths and safe path movement prevent monotony
5. **Efficient**: O(1) generation per lane, no storing entire level
6. **Procedural**: Same seed produces same pattern (replayability)

### Camera & Fail State Update

```cpp
void Game::updateCameraAndFailState(float deltaTime)
{
    glm::vec3 playerBasePos = player.getBasePosition();
    
    float idealCameraTrackZ = cameraTrackZ - Config::CAMERA_AUTO_SCROLL_SPEED * deltaTime;
    if (playerBasePos.z < idealCameraTrackZ)
        idealCameraTrackZ = playerBasePos.z;
    cameraTrackZ = idealCameraTrackZ;
    
    float lerpXY = 1.0f - std::exp(-Config::CAMERA_SMOOTH_SPEED_XY * deltaTime);
    float lerpZ = 1.0f - std::exp(-Config::CAMERA_SMOOTH_SPEED_Z * deltaTime);

    smoothedCameraTarget.x = glm::mix(smoothedCameraTarget.x, playerBasePos.x, lerpXY);
    smoothedCameraTarget.y = glm::mix(smoothedCameraTarget.y, playerBasePos.y, lerpXY);
    smoothedCameraTarget.z = glm::mix(smoothedCameraTarget.z, cameraTrackZ, lerpZ);

    camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);

    if (player.getPosition().z > cameraTrackZ + Config::CAMERA_BACKWARD_DEATH_DISTANCE) {
        if (!player.getIsDead()) {
            player.setDead(true);
            deathPosition = player.getPosition();
            hasWaterDeath = false;
        }
        state = GAME_STATE_GAME_OVER;
        SaveManager::saveData(totalCoins, highScore);
    }
}
```

**Explanation:**
- Camera follows player using exponential lerp (smooth interpolation)
- Different lerp speeds for X/Y (1.0) vs Z (1.0)
- If player moves 5+ units backward past camera, triggers death
- Updates camera matrices for 3D projection
- Saves data when game over

### Infinite Lane Maintenance

```cpp
void Game::maintainInfiniteLanes()
{
    glm::vec3 playerPos = player.getPosition();
    float forwardReferenceZ = std::min(playerPos.z, cameraTrackZ);

    while (currentGenerationZ > forwardReferenceZ - Config::LANE_GENERATION_BUFFER_AHEAD)
        generateLaneBlock();

    float pruneBehindZ = playerPos.z + Config::LANE_CLEANUP_BUFFER_BEHIND;
    int removeCount = 0;
    while (removeCount < (int)lanes.size() &&
           lanes[removeCount].getZPosition() > pruneBehindZ)
        removeCount++;
    if (removeCount > 0)
        lanes.erase(lanes.begin(), lanes.begin() + removeCount);
}
```

**Explanation:**
- **Generation**: Creates new lanes if generated point is less than 25 units ahead
- **Cleanup**: Removes lanes more than 12 units behind player to save memory
- Keeps lane vector small while maintaining infinite scrolling feel

### Update Function

```cpp
void Game::update(float deltaTime)
{
    for (auto &lane : lanes)
        lane.update(deltaTime);

    updateDayNightCycle(deltaTime);

    if (state == GAME_STATE_MAIN_MENU || state == GAME_STATE_CHARACTER_SELECT || state == GAME_STATE_START_SCREEN) {
        smoothedCameraTarget = player.getBasePosition();
        camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
        return;
    }

    if (state == GAME_STATE_GAME_OVER) {
        player.update(deltaTime);
        camera.setTargetRadius(Config::DEAD_ZOOM_RADIUS);
        camera.setLerpSpeed(Config::DEAD_ZOOM_SPEED);
        bool useWater = hasWaterDeath || hasStreamDeath;
        glm::vec3 trackPos = useWater ? deathPosition : player.getPosition();
        float snapSpeed = useWater ? 0.14f : 0.05f;

        smoothedCameraTarget.x = glm::mix(smoothedCameraTarget.x, trackPos.x, snapSpeed);
        smoothedCameraTarget.z = glm::mix(smoothedCameraTarget.z, trackPos.z, snapSpeed);
        camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
        return;
    }

    player.update(deltaTime);
    checkCollisions(deltaTime);

    if (state != GAME_STATE_PLAYING)
        return;

    updateCameraAndFailState(deltaTime);
    maintainInfiniteLanes();

    int lanesMoved = static_cast<int>(std::round((startZ - player.getBasePosition().z) / Config::CELL_SIZE));
    if (lanesMoved > score) {
        score = lanesMoved;
        if (score > highScore) {
            highScore = score;
        }
    }
}
```

**Game State Machine - State Routing:**

**Algorithm Type**: State-conditional dispatch pattern

**State Definitions:**
```
enum GameState {
    GAME_STATE_MAIN_MENU,        // Title screen
    GAME_STATE_CHARACTER_SELECT,  // Pick player character
    GAME_STATE_START_SCREEN,      // "Press Start" screen
    GAME_STATE_PLAYING,           // Active gameplay
    GAME_STATE_GAME_OVER          // Death screen with restart options
};
```

**Step 1: Always Update Lanes & Time**
```cpp
for (auto &lane : lanes)
    lane.update(deltaTime);

updateDayNightCycle(deltaTime);
```
- Runs every state
- Obstacles move regardless of state
- Day/night cycle affects lighting everywhere

**Step 2: Menu States (Non-Gameplay)**
```cpp
if (state == GAME_STATE_MAIN_MENU || 
    state == GAME_STATE_CHARACTER_SELECT || 
    state == GAME_STATE_START_SCREEN) {
    smoothedCameraTarget = player.getBasePosition();
    camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
    return;  // Skip gameplay processing
}
```

**Menu State Behavior:**
```
No gameplay updates in these states
- Player not moving (stationary)
- No collision checks running
- Camera simply tracks player (stationary)
- Graphics render, but nothing interactive

Transitions:
  MAIN_MENU + (keyboard select) → CHARACTER_SELECT
  CHARACTER_SELECT + (enter key) → START_SCREEN  
  START_SCREEN + (space bar) → PLAYING
```

**Step 3: Game Over State (Death Cam)**
```cpp
if (state == GAME_STATE_GAME_OVER) {
    player.update(deltaTime);  // Continue death animation
    camera.setTargetRadius(Config::DEAD_ZOOM_RADIUS);
    camera.setLerpSpeed(Config::DEAD_ZOOM_SPEED);
    
    bool useWater = hasWaterDeath || hasStreamDeath;
    glm::vec3 trackPos = useWater ? deathPosition : player.getPosition();
    float snapSpeed = useWater ? 0.14f : 0.05f;

    smoothedCameraTarget.x = glm::mix(smoothedCameraTarget.x, trackPos.x, snapSpeed);
    smoothedCameraTarget.z = glm::mix(smoothedCameraTarget.z, trackPos.z, snapSpeed);
    camera.update(deltaTime, windowWidth, windowHeight, smoothedCameraTarget);
    return;
}
```

**Game Over Processing:**

```
Death Sequence:
  1. Collision triggers (train/car/water)
  2. player.setDead(true) called
  3. state = GAME_STATE_GAME_OVER
  4. Next frame: update() enters GAME_OVER state
  
Camera Behavior (Death Cam):
  Normal zoom: radius = 12, slow lerp
  Death zoom: radius = 6, faster lerp (0.14 snap speed)
  
  Result: Camera rapidly zooms in to death location
  
Death Position Tracking:
  Water death: 
    useWater = true
    trackPos = deathPosition (exact death spot)
    snapSpeed = 0.14 (fast zoom)
  
  Squish death:
    useWater = false
    trackPos = player.getPosition() (current position)
    snapSpeed = 0.05 (slower follow)

Visual Effect:
  t=0: Player dies
  t=0-0.5s: Camera rapidly zooms to death spot (water) OR
            slowly follows squished player (train/car)
  Particles render at death location
  Player sees dramatic camera movement
```

**Step 4: Playing State (Full Gameplay)**
```cpp
player.update(deltaTime);
checkCollisions(deltaTime);

if (state != GAME_STATE_PLAYING)
    return;

updateCameraAndFailState(deltaTime);
maintainInfiniteLanes();

// Score tracking
int lanesMoved = static_cast<int>(
    std::round((startZ - player.getBasePosition().z) / Config::CELL_SIZE)
);
if (lanesMoved > score) {
    score = lanesMoved;
    if (score > highScore) {
        highScore = score;
    }
}
```

**PLAYING State Logic:**

```
Execution Flow Every Frame:
  1. player.update() - advance jump animation, position
  2. checkCollisions() - detect hazards
     - If collision: state → GAME_OVER, return
  3. Check state still PLAYING (collision might have changed it)
  4. If still PLAYING:
     - updateCameraAndFailState() - track camera to player
     - maintainInfiniteLanes() - generate lanes ahead, delete behind
     - Calculate score (lanes passed)
     - Check if beat high score
```

**Score Calculation Algorithm:**

```
Initial: startZ = 0 (starting position)
Current: player.getBasePosition().z = -15.5 (moved forward)

Distance: 0 - (-15.5) = 15.5 units
Lane width: CELL_SIZE = 1.0 unit
Lanes passed: 15.5 / 1.0 = 15.5 → round to 16 lanes

score = 16
highScore = max(highScore, 16)

Example progression:
  t=0s: score = 0, player at Z=0
  t=10s: Z=-26, score = 26
  t=20s: Z=-52, score = 52
  
If highScore was 100 and player reaches 105:
  highScore updated to 105
  Next game: highScore displays 105
  Data saved to savedata.dat
```

**State Transition Diagram:**

```
                ↓
        [MAIN_MENU]
         ↓       ↑
         │ ESC   │ Back button
         ↓       ↑
    [CHARACTER_SELECT]
         ↓       ↑
         │ ESC   │ Back button
         ↓       ↑
    [START_SCREEN]
         ↓       ↑
         │ Space │ Restart button
         ↓       ↑
    [PLAYING]────→[GAME_OVER]
         ↓        → Restart (R key)
        ...       → Menu (M key)

Key Input Handlers:
  ESC: Always exits to main menu (from any state)
  In PLAYING: W/A/S/D = jump, R/M = restart/menu
  In GAME_OVER: R = restart, M = menu
```

### Collision Detection

```cpp
void Game::checkCollisions(float deltaTime)
{
    glm::vec3 playerPos = player.getPosition();
    glm::vec3 playerSize = player.getSize();
    glm::vec3 playerBasePos = player.getBasePosition();

    bool onLog = false;
    bool onLilypad = false;
    Lane *currentLane = nullptr;

    for (auto &lane : lanes) {
        if (std::abs(lane.getZPosition() - playerPos.z) < Config::CELL_SIZE / 2.0f) {
            currentLane = &lane;
            break;
        }
    }
    if (!currentLane) return;
```

**Detailed Explanation - Per-Frame Collision Detection System:**

**Algorithm Type**: Multi-layer spatial partitioning with early exits

**Purpose**: Check all hazards each frame and trigger appropriate death/movement states

**Inputs:**
- `playerPos`: Position during jump animation (mid-air during jump, ground when landed)
- `playerSize`: Collision bounding box (0.8×0.8×0.8 units, padded to 0.64)
- `playerBasePos`: Stable ground position (what we save for next spawn)
- `deltaTime`: Frame time for physics (not used in collision, only death animations)

**Step 1: Find Current Lane**
```cpp
for (auto &lane : lanes) {
    if (std::abs(lane.getZPosition() - playerPos.z) < Config::CELL_SIZE / 2.0f) {
        currentLane = &lane;
        break;
    }
}
if (!currentLane) return;
```

**Lane Detection Algorithm:**

```
Lane Grid (vertical strips, each 1 unit tall):
  Z Position:  0    -1    -2    -3    -4   ...
  Lane:       [0]  [1]  [2]  [3]  [4]  ...
  Type:      GRS  GRS  GRS  ROAD ROAD ...
  Obstacles:  -    -    -   [cars][cars] ...

Player Position: Z = -3.2

Check each lane:
  |0 - (-3.2)| = 3.2 > 0.5 (threshold) ✗
  |-1 - (-3.2)| = 2.2 > 0.5 ✗
  |-2 - (-3.2)| = 1.2 > 0.5 ✗
  |-3 - (-3.2)| = 0.2 < 0.5 ✓ MATCH!
  
  currentLane points to lane 3 (ROAD lane with cars)
  break (exit early, don't check remaining lanes)
```

**Why ±0.5 Tolerance:**
```
Config::CELL_SIZE = 1.0 (lanes are 1 unit apart)
Tolerance = 1.0 / 2 = 0.5

Rationale:
  Player at Z = -3.0 (exactly on lane edge):
    Could be ending lane 2 or starting lane 3
    Tolerance ±0.5 puts them in lane 3
  
  Player at Z = -3.2 (during jump toward lane 3):
    Distance from lane 3 = 0.2 < 0.5
    Confirmed: in lane 3 landing zone
  
  Player at Z = -2.8 (bounced back):
    Distance from lane 3 = 0.2 < 0.5
    Still consider lane 3 (forgiveness)
```

**Early Exit Optimization:**
```cpp
if (!currentLane) return;
```

Bailed if:
- Player somehow off all lanes (shouldn't happen)
- No valid lane found (edge case)
- Prevents null pointer crash in subsequent collision checks
```

**Explanation:**
- Identifies which lane player is currently on
- Uses Z position to find matching lane
- Exits early if no lane found (shouldn't happen)

### Train Collision

```cpp
    for (auto &lane : lanes) {
        if (lane.getType() != LANE_RAIL) continue;
        if (std::abs(lane.getZPosition() - playerPos.z) > Config::CELL_SIZE * 2.0f) continue;

        for (auto &obs : lane.getObstacles()) {
            if (obs.getType() != OBSTACLE_TRAIN || !obs.getIsActive()) continue;

            glm::vec3 obsPos = obs.getPosition();
            glm::vec3 obsSize = obs.getSize();
            bool hitX = std::abs(playerBasePos.x - obsPos.x) < (playerSize.x + obsSize.x) * 0.5f * Config::HITBOX_PADDING;
            bool hitZ = std::abs(playerPos.z - obsPos.z) < Config::CELL_SIZE * 0.9f;

            if (hitX && hitZ) {
                player.setDead(true);
                deathPosition = playerPos;
                hasWaterDeath = false;
                state = GAME_STATE_GAME_OVER;
                SaveManager::saveData(totalCoins, highScore);
                return;
            }
        }
    }
```

**Detailed Explanation - Train Collision Detection:**

**Scope**: Check all rails within ±2 lanes of player for incoming trains

**Algorithm Type**: Multi-lane lookahead with AABB collision

**Why Check ±2 Lanes:**
```
Player on lane 3
Check lanes: 1, 2, 3, 4, 5 (±2 range)
Reason: Train moves 25 units/sec at 120 FPS
  Per frame displacement = 25 * (1/120) ≈ 0.21 units
  In one frame, train advances 0.21 units toward player
  Need lookahead to predict collision next frame

Distance threshold = CELL_SIZE * 2 = 2 units
At 25 units/sec, train takes 0.08 seconds to cross 2 lanes
Enough time for collision detection before impact
```

**Step 1: Lane Type Filter**
```cpp
if (lane.getType() != LANE_RAIL) continue;
```
- Skip non-rail lanes (trains only on rails)
- Optimization: avoid checking road, grass, river for trains
- Result: Only process 2 rail lanes

**Step 2: Distance Check**
```cpp
if (std::abs(lane.getZPosition() - playerPos.z) > Config::CELL_SIZE * 2.0f) continue;
```
- If rail lane > 2 units away: skip it
- Only check nearby lanes for performance

**Step 3: Iterate Active Trains & Collision**
```cpp
for (auto &obs : lane.getObstacles()) {
    if (obs.getType() != OBSTACLE_TRAIN || !obs.getIsActive()) continue;
    
    glm::vec3 obsPos = obs.getPosition();
    glm::vec3 obsSize = obs.getSize();
    bool hitX = std::abs(playerBasePos.x - obsPos.x) < (playerSize.x + obsSize.x) * 0.5f * Config::HITBOX_PADDING;
    bool hitZ = std::abs(playerPos.z - obsPos.z) < Config::CELL_SIZE * 0.9f;
    
    if (hitX && hitZ) {
        player.setDead(true);
        // ... game over sequence
    }
}
```

### Current Lane Obstacles

```cpp
    for (auto &obs : currentLane->getObstacles()) {
        if (!obs.getIsActive()) continue;

        ObstacleType obsType = obs.getType();
        glm::vec3 obsPos = obs.getPosition();
        glm::vec3 obsSize = obs.getSize();

        bool isColliding = false;
        if (obsType == OBSTACLE_LOG || obsType == OBSTACLE_LILYPAD) {
            isColliding = Collision::checkAABB(
                playerBasePos, playerSize,
                obsPos, obsSize);
        } else {
            isColliding = Collision::checkAABB(
                playerPos, playerSize,
                obsPos, obsSize);
        }

        if (isColliding) {
            if (obsType == OBSTACLE_CAR || obsType == OBSTACLE_TRAIN) {
                player.setDead(true);
                deathPosition = playerPos;
                hasWaterDeath = false;
                state = GAME_STATE_GAME_OVER;
                SaveManager::saveData(totalCoins, highScore);
                return;
            }
```

**Detailed Explanation - Per-Lane Obstacle Collision:**

**Algorithm Type**: Polymorphic collision with type-specific position selection

**Purpose**: Check all obstacles in current lane, handle collisions based on type

**Step 1: Type-Specific Position Selection**
```cpp
if (obsType == OBSTACLE_LOG || obsType == OBSTACLE_LILYPAD) {
    // Use GROUND position (player feet level)
    isColliding = Collision::checkAABB(
        playerBasePos, playerSize,    // Ground-level player
        obsPos, obsSize);             // Floating object
} else {
    // Use MID-AIR position (player current jump position)
    isColliding = Collision::checkAABB(
        playerPos, playerSize,        // Current jump position
        obsPos, obsSize);             // Ground object
}
```

**Why Different Positions:**
```
Logs/Lilypads (water, Y ≈ -0.1):
  Use playerBasePos (feet level)
  Physics: Player stands ON surface
  Detection: At ground level where feet touch
  
Cars (road, Y ≈ 0):
  Use playerPos (mid-jump position)
  Physics: Mid-air collision during jump arc
  Detection: Anywhere in jump trajectory
  Example: Player jumping over car might still collide mid-arc
```

**Step 2: Car/Train Instant Death**
```cpp
if (obsType == OBSTACLE_CAR || obsType == OBSTACLE_TRAIN) {
    player.setDead(true);
    deathPosition = playerPos;
    hasWaterDeath = false;
    state = GAME_STATE_GAME_OVER;
    SaveManager::saveData(totalCoins, highScore);
    return;
}
```

### Log Velocity Transfer & Stream Detection

```cpp
            else if (obsType == OBSTACLE_LOG) {
                onLog = true;
                obs.setSinking(true);
                player.applyLogVelocity(obs.getSpeed(), deltaTime);

                if (std::abs(obsPos.x) > Config::LOG_STREAM_TRIGGER_X) {
                    obs.setFastStream(true);
                }

                if (std::abs(playerPos.x) > Config::BOUNDARY_X + 0.5f) {
                    const float waterSurface = -Config::CELL_SIZE * 0.1f;
                    player.triggerWaterDeath(waterSurface);
                    deathPosition = playerPos;
                    hasWaterDeath = true;
                    hasStreamDeath = true;
                    state = GAME_STATE_GAME_OVER;
                    SaveManager::saveData(totalCoins, highScore);
                    return;
                }
            }
```

**Detailed Explanation - Log Interactions & Stream Death:**

**Step 1: Mark Safe From Water**
```cpp
onLog = true;
```
- Sets flag preventing drowning check later
- Water hazard skipped: `if (!onLog) water_check()`

**Step 2: Visual Feedback**
```cpp
obs.setSinking(true);
```
- Log visibly depresses (lerps down -0.07 units)
- Player feels weight of standing
- Effect: Log appears to sink under weight

**Step 3: Transfer Log Velocity**
```cpp
player.applyLogVelocity(obs.getSpeed(), deltaTime);
```

```
Log speed: -5.0 units/sec (moving left)
Player per-frame: -5.0 * (1/120) = -0.0417 units

Result: Player swept leftward with log
Over 60 frames (0.5 sec): -2.5 units displacement
Seamless transport across water
```

**Step 4: Stream Zone Detection**
```cpp
if (std::abs(obsPos.x) > Config::LOG_STREAM_TRIGGER_X) {
    obs.setFastStream(true);
}
```

```
Log positions: x ∈ [-15, +15] (visible screen)
Stream trigger: |x| > 12 (foam boundary)

When log crosses into ±12:
  Acceleration phase begins
  Speed increases exponentially
  Player experience: log suddenly speeds up
```

**Step 5: Stream Death Boundary Check**
```cpp
if (std::abs(playerPos.x) > Config::BOUNDARY_X + 0.5f) {
    // BOUNDARY_X = ±12, check > ±12.5
    player.triggerWaterDeath(waterSurface);
    hasStreamDeath = true;
}
```

```
Scenario: Player on accelerating log
  1. Log reaches stream zone (x=12)
  2. Accelerates exponentially (6x)
  3. Reaches x=20 (beyond boundary)
  4. Player still standing on log
  5. Water death triggered
  6. hasStreamDeath=true (death type metadata)
```

### Water/Lilypad Hazard Check

```cpp
    LaneType laneType = currentLane->getType();
    if ((laneType == LANE_RIVER || laneType == LANE_LILYPAD) && !player.getIsJumping() && !onLog && !onLilypad) {
        const float waterSurface = -Config::CELL_SIZE * 0.1f;
        player.triggerWaterDeath(waterSurface);
        deathPosition = playerPos;
        hasWaterDeath = true;
        hasStreamDeath = false;
        state = GAME_STATE_GAME_OVER;
        SaveManager::saveData(totalCoins, highScore);
    }
}
```

**Detailed Explanation - Multi-Condition Drowning Logic:**

**Algorithm Type**: AND-gate safety zone detector

**Drowning Triggered ONLY IF ALL TRUE:**
```
(laneType == LANE_RIVER || LANE_LILYPAD)  ← On water lane
&& !player.getIsJumping()                  ← Not mid-jump
&& !onLog                                  ← Not on floating log
&& !onLilypad                              ← Not on lily pad
```

**Safety Zone: Jump Arc**
```
Player jumping over water:
  playerPos.y = 0.75 (at peak)
  player.getIsJumping() = true
  Water check: skipped!
  Result: Safe during jump (over water)
```

**Safety Zone: Log**
```
Player on log:
  onLog = true (set during collision with log)
  Water check: skipped!
  Result: Safe while carried by log
```

**Safety Zone: Lilypad**
```
Player on lilypad:
  onLilypad = true (set during collision with lilypad)
  Water check: skipped!
  Result: Safe standing on solid platform
```

**Drown Condition: Fall in Empty Water**
```
Scenario:
  1. Player jumps toward river
  2. All logs moved away
  3. Player lands on empty water (no log)
  4. player.getIsJumping() = false (jump complete)
  5. onLog = false (no collision)
  6. onLilypad = false (no collision)
  7. All conditions met → Water death triggered
```

### World Boundary Rendering

```cpp
void Game::renderWorldBoundaries()
{
    const glm::vec3 playerPos = player.getPosition();
    float snappedZ = std::round(playerPos.z / Config::CELL_SIZE) * Config::CELL_SIZE;
    
    const float zStart = snappedZ + 9.0f * Config::CELL_SIZE;
    const float zEnd = snappedZ - 38.0f * Config::CELL_SIZE;
    const float step = Config::CELL_SIZE;

    struct LaneInfo {
        LaneType type;
        float logFlowDir;
    };
    std::map<float, LaneInfo> laneMap;
    for (const auto &lane : lanes) {
        LaneType ltype = lane.getType();
        float lfd = 0.0f;
        if (ltype == LANE_RIVER || ltype == LANE_LILYPAD) {
            for (const auto &obs : lane.getObstacles()) {
                if (obs.getIsActive()) {
                    lfd = (obs.getSpeed() >= 0.0f) ? 1.0f : -1.0f;
                    break;
                }
            }
        }
        laneMap[lane.getZPosition()] = {ltype, lfd};
    }
```

**Explanation:**
- Snaps Z to grid to prevent camera jitter during smooth jumping
- Renders ~47 lane slices (9 ahead, 38 behind)
- Pre-builds O(1) lookup map instead of O(n) searches per slice (~3-5% optimization)
- Tracks log flow direction for visual indicators

### Mountain Portal Optimization

```cpp
    const int nSlices = static_cast<int>(slices.size());
    for (int i = 0; i < nSlices; i++) {
        float z = slices[i].z;
        LaneType ltype = slices[i].ltype;
        float lfd = slices[i].logFlowDir;
        bool isTunnel = (ltype == LANE_ROAD || ltype == LANE_RAIL);

        bool isPortalFace = true;

        if (isTunnel) {
            LaneType prev = (i > 0) ? slices[i - 1].ltype : LANE_GRASS;
            LaneType next = (i < nSlices - 1) ? slices[i + 1].ltype : LANE_GRASS;

            bool isEntrance = (prev != ltype);
            isPortalFace = isEntrance;
        }

        renderer.drawMountainSection(z, ltype, lfd, isPortalFace);
    }
```

**Explanation:**
- Draws mountain walls for each lane
- Tunnel lanes (road/rail) only show entrance portals for better performance
- Prevents drawing mountain for every lane of a 3-lane road block

### Foam Rendering

```cpp
    for (const auto &lane : lanes) {
        LaneType lt = lane.getType();
        if (lt != LANE_RIVER && lt != LANE_LILYPAD) continue;
        float z = lane.getZPosition();
        if (z > playerPos.z + 10.0f || z < playerPos.z - 38.0f) continue;

        float foamY = -0.06f;
        renderer.drawFoam({-Config::BOUNDARY_X + 0.25f, foamY, z}, 1.2f, 0.9f);
        renderer.drawFoam({Config::BOUNDARY_X - 0.25f, foamY, z}, 1.2f, 0.9f);
    }

    renderer.drawBackWall();
}
```

**Explanation:**
- Draws foam barriers at water lane boundaries (safe/unsafe transition)
- Only renders visible slices (±10 units from player)
- Back wall completes the scene boundary

### Day/Night Cycle

```cpp
void Game::updateDayNightCycle(float deltaTime)
{
    currentGameTime += deltaTime;
    float cycleTime = std::fmod(currentGameTime * Config::TIME_SPEED, 1.0f);
    
    sunAngle = (cycleTime - 0.5f) * 3.14159f;
    renderer.updateLighting(currentGameTime);
    
    const float HORIZON_ANGLE = 3.14159f / 4.0f;
    float sunAngleMagnitude = std::abs(sunAngle);
    bool isDayTime = (sunAngleMagnitude < HORIZON_ANGLE);
    renderer.setNightMode(!isDayTime);
}
```

**Explanation:**
- `cycleTime` normalized 0-1 over cycle duration
- `sunAngle` maps to full -π to +π arc (sun below/above horizon)
- At |sunAngle| > π/4: sun below horizon = night mode
- Updates renderer lighting (sun position, colors)

### Shadow Rendering

```cpp
void Game::renderShadows()
{
    if (state != GAME_STATE_PLAYING) return;
    
    float sunAngleMagnitude = std::abs(sunAngle);
    float shadowFadeFactor = renderer.getShadowFadeFactor(sunAngle);
    
    if (shadowFadeFactor <= 0.0f) return;
    
    glPushMatrix();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_TEXTURE_2D);
    
    glm::vec3 playerPos = player.getPosition();
    glm::vec3 playerSize = player.getSize();
    renderer.drawCharacterShadow(playerPos, playerSize, sunAngle, sunAngleMagnitude, shadowFadeFactor, LANE_GRASS);
    
    float playerZ = playerPos.z;
    const float SHADOW_RENDER_DISTANCE = 20.0f;
    
    for (auto &lane : lanes) {
        LaneType laneType = lane.getType();
        
        for (auto &obs : lane.getObstacles()) {
            if (!obs.getIsActive()) continue;
            
            glm::vec3 obsPos = obs.getPosition();
            float distanceFromPlayer = std::abs(obsPos.z - playerZ);
            if (distanceFromPlayer > SHADOW_RENDER_DISTANCE) continue;
            
            ObstacleType obsType = obs.getType();
            if (obsType == OBSTACLE_CAR || obsType == OBSTACLE_TRAIN || 
                obsType == OBSTACLE_LOG || obsType == OBSTACLE_LILYPAD) {
                glm::vec3 obsSize = obs.getSize();
                renderer.drawObstacleShadow(obsPos, obsSize, sunAngle, sunAngleMagnitude, shadowFadeFactor, laneType);
            }
        }
```

**Explanation:**
- Skips shadows if not daylight or sun angle too low
- Sets GL state for transparent shadow rendering
- Visibility culling: only renders shadows ±20 units from player
- Renders player shadow + obstacle shadows
- Handles trees and rocks on grass lanes

### Main Render Function

```cpp
void Game::render()
{
    renderer.prepareFrame();
    camera.apply();

    lastFrameTime = glutGet(GLUT_ELAPSED_TIME);

    renderWorldBoundaries();

    for (auto &lane : lanes)
        lane.render(renderer, sunAngle, lastFrameTime);

    renderShadows();

    if (state == GAME_STATE_MAIN_MENU || state == GAME_STATE_START_SCREEN || state == GAME_STATE_CHARACTER_SELECT) {
        glm::vec3 pos = player.getPosition();
        int currentTime = lastFrameTime;
        float timeSinceClick = (currentTime - lastClickTime) / 1000.0f;

        float wobble = 0.0f;
        if (eggClicks > 0 && timeSinceClick < 0.25f) {
            wobble = std::sin(timeSinceClick * 50.0f) * 0.3f;
            wobble *= (1.0f - timeSinceClick / 0.25f);
        }

        glPushMatrix();
        glTranslatef(pos.x, pos.y + 0.6f, pos.z);
        glRotatef(wobble * 45.0f, 0, 0, 1);
        glScalef(0.5f, 0.5f, 0.5f);
        renderer.drawEgg(eggClicks);
        glPopMatrix();
    }
    else if (state == GAME_STATE_PLAYING) {
        int currentTime = lastFrameTime;
        float timeSinceStart = (currentTime - lastClickTime) / 1000.0f;
        float spawnDuration = 0.4f;

        if (timeSinceStart < spawnDuration) {
            float t = timeSinceStart / spawnDuration;
            float t1 = t - 1.0f;
            float scale = t1 * t1 * (2.5f * t1 + 1.5f) + 1.0f;
            if (scale < 0.0f) scale = 0.0f;

            glm::vec3 pos = player.getPosition();
            glPushMatrix();
            glTranslatef(pos.x, pos.y, pos.z);
            glScalef(scale, scale, scale);
            glTranslatef(-pos.x, -pos.y, -pos.z);
            player.render(renderer);
            glPopMatrix();
        } else {
            player.render(renderer);
        }
    }
    else if (state == GAME_STATE_GAME_OVER) {
        player.render(renderer);
    }

    camera.renderOverlay(windowWidth, windowHeight);
    renderUIOverlay();
}
```

**Detailed Explanation - Complete Rendering Pipeline:**

**Algorithm Type**: Multi-layer rendering with state-based branches

**Purpose**: Execute all 3D/2D rendering for one frame, in correct order

**Step 1: Prepare Framebuffer**
```cpp
renderer.prepareFrame();
```

**Frame Setup Sequence:**
```
1. glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
   - Clear color buffer (erase last frame)
   - Clear depth buffer (reset distance calculations)
   
2. glClearColor() - Set background color (usually sky blue)
   
3. glEnable(GL_DEPTH_TEST)
   - Enable depth testing (objects closer to camera drawn on top)
   
4. glMatrixMode(GL_MODELVIEW)
   glLoadIdentity()
   - Reset transformation matrix to identity
   - All objects start at world origin
```

**Step 2: Apply Camera**
```cpp
camera.apply();
```

**Camera Application:**
```
Uses previously calculated view matrix from camera.update()

Matrix Operations:
  gluLookAt(eyePos, targetPos, upVec)
  ↓
  Transforms world coordinate system
  ↓
  Everything draws relative to camera position
  
Example (orbital camera):
  Eye: [7.34, 6.0, 2.34] (spherical to Cartesian)
  Target: [0, 0, -20] (player position)
  Up: [0, 1, 0]
  
  Result: 3D world appears rotated/moved per camera
```

**Step 3: Capture Current Time**
```cpp
lastFrameTime = glutGet(GLUT_ELAPSED_TIME);
```

**Timing For Animations:**
```
Returns milliseconds since GLUT initialization

Uses:
  - Egg wobble animation (sine wave based on elapsed time)
  - Player spawn scale animation (interpolation)
  - Water death particle spawn (time-based lifecycle)
  - Day/night cycle (sun angle from elapsed time)
```

**Step 4: Render World Boundaries**
```cpp
renderWorldBoundaries();
```

**Boundary Visuals:**
```
Renders mountain/forest walls at edges
  Left wall: x = -12
  Right wall: x = +12
  Front wall: z = 0 (back of screen)
  
Visual: Large brown/green cube models forming barriers
Purpose: Prevents player from seeing off-world
```

**Step 5: Render Lanes (Primary 3D Content)**
```cpp
for (auto &lane : lanes)
    lane.render(renderer, sunAngle, lastFrameTime);
```

**Lane Rendering Loop:**
```
For each lane in view (typically 12-15 lanes visible):
  
  1. Ground quad (grass/road/water/rails)
     - Colored surface at correct Z position
     - Dimensions: X ∈ [-15, +15], Z = lane.Z
  
  2. Obstacles on lane:
     - Cars (road lanes): gray/red rectangles
     - Trains (rail lanes): long train models with wheels
     - Logs (water lanes): brown cylinders
     - Lilypads (water lanes): green circles
     - All at correct X/Z position, moving per obstacle.speed
  
  3. Coins (if present):
     - Spinning 3D circles
     - Yellow with rotation: angle += deltaTime * 180°/sec
  
  4. Lane-specific features:
     - Water ripples (animated texture shifts)
     - Signal posts (red/green lights for trains)
     - Track markers (visual guides)
```

**Step 6: Render Shadows**
```cpp
renderShadows();
```

**Shadow Rendering:**
```
For each renderable object:
  1. Get sun angle (0° = sunrise, 90° = overhead, 180° = sunset)
  2. Calculate shadow vector (away from sun)
  3. Draw dark rectangle on ground
     - Position: object position + shadow offset
     - Size: based on object size and sun height
     - Darkness: varies with sun angle (noon = darker)
  
  Result: Objects appear grounded with realistic shadow
```

**Step 7: Character Rendering (State-Dependent)**

**Menu States (Wobbling Egg):**
```cpp
if (state == GAME_STATE_MAIN_MENU || ...CHARACTER_SELECT) {
    float wobble = std::sin(timeSinceClick * 50.0f) * 0.3f;
    wobble *= (1.0f - timeSinceClick / 0.25f);  // Decay over 0.25s
    
    glRotatef(wobble * 45.0f, 0, 0, 1);
    renderer.drawEgg(eggClicks);
}
```

**Menu Character Effect:**
```
When player clicks on egg:
  t=0: wobble = sin(0) * 0.3 = 0 (no rotation)
  t=0.125s: wobble = sin(6.25) * 0.3 * 0.5 (peak bounce)
  t=0.25s: wobble = sin(12.5) * 0.3 * 0 (full decay)
  
Result: Egg spins briefly when clicked, decays smoothly
```

**Playing State (Spawn Animation):**
```cpp
float t = timeSinceStart / spawnDuration;  // 0 to 1 over 0.4s
float scale = t1 * t1 * (2.5f * t1 + 1.5f) + 1.0f;
player.render(renderer);
```

**Ease-Out Bounce Formula:**
```
This is an ease-out cubic formula (object grows then settles)

At t=0: scale = 1.0 (hidden or tiny)
At t=0.2: scale ≈ 1.15 (growing)
At t=0.4: scale = 1.0 (settled at normal size)

Result: Player pops into world with smooth bounce effect
```

**Game Over State:**
```cpp
player.render(renderer);
```
- Simple render at death position
- Camera already zoomed in via death cam

**Step 8: Render Camera Overlay Elements**
```cpp
camera.renderOverlay(windowWidth, windowHeight);
```

**Overlay Elements:**
```
Switches to 2D/orthographic projection

Renders:
  - Camera preset indicators (which camera mode active)
  - Debug information (if enabled)
  - HUD elements specific to camera (distance, angle)
```

**Step 9: Render UI Overlay**
```cpp
renderUIOverlay();
```

**UI Layers:**
```
Switches to full 2D orthographic mode
X: 0 to 800 pixels, Y: 0 to 600 pixels

Renders in order (back to front):
  1. Background panels (semi-transparent dark rectangles)
  2. Score text (white text, top-left corner)
  3. FPS counter (debug, top-right)
  4. Menu buttons (if in menu state)
     - Character portraits with names
     - Start/Resume/Restart buttons
     - Settings buttons
  5. Game over screen
     - "GAME OVER" text (large, red)
     - Score display
     - "Press R to Restart" instructions
     - "Press M for Menu" instructions

All UI drawn AFTER 3D content (on top)

### Input Handling

```cpp
void Game::onKeyPress(unsigned char key)
{
    preGameManager.onKeyPress(key, state, selectedCharacterIndex, player);

    if (state != GAME_STATE_PLAYING) return;

    float dx = 0.0f, dz = 0.0f;
    if (key == 'w' || key == 'W') dz = -1.0f;  // Forward
    if (key == 's' || key == 'S') dz = 1.0f;   // Backward
    if (key == 'a' || key == 'A') dx = -1.0f;  // Left
    if (key == 'd' || key == 'D') dx = 1.0f;   // Right

    if (dx == 0.0f && dz == 0.0f) {
        if (key == 'v' || key == 'V') camera.cyclePreset();
        if (key == 'c' || key == 'C') camera.toggleLock();
        return;
    }

    glm::vec3 currentPos = player.getPosition();
    glm::vec3 nextPos = currentPos + glm::vec3(dx * Config::CELL_SIZE,
                                               0.0f,
                                               dz * Config::CELL_SIZE);

    if (std::abs(nextPos.x) >= Config::BOUNDARY_X) return;
    if (nextPos.z > Config::BOUNDARY_BACK_Z) return;

    Lane *targetLane = nullptr;
    for (auto &lane : lanes) {
        if (std::abs(lane.getZPosition() - nextPos.z) < Config::CELL_SIZE / 2.0f) {
            targetLane = &lane;
            break;
        }
    }

    bool blocked = false;
    if (targetLane && targetLane->getType() == LANE_GRASS) {
        for (auto &d : targetLane->decorations) {
            float sz = (d.type == 0) ? 0.6f : 0.5f;
            if (std::abs(nextPos.x - d.position.x) < sz &&
                std::abs(nextPos.z - d.position.z) < sz) {
                blocked = true;
                break;
            }
        }
    }

    if (!blocked) player.move(dx, dz);
}
```

**Detailed Explanation - Input Processing & Jump Validation:**

**Algorithm Type**: Jump targeting with collision pre-checking

**Purpose**: Validate jump destination before allowing move (prevents wall clipping)

**Step 1: Movement Vector Calculation**
```cpp
float dx = 0.0f, dz = 0.0f;
if (key == 'w' || key == 'W') dz = -1.0f;  // Forward (toward positive score)
if (key == 's' || key == 'S') dz = 1.0f;   // Backward
if (key == 'a' || key == 'A') dx = -1.0f;  // Left
if (key == 'd' || key == 'D') dx = 1.0f;   // Right
```

**Movement Grid:**
```
Player moves on grid basis
Each press = 1 cell = Config::CELL_SIZE = 1.0 unit

Example keyboard sequence:
  Press W: dz = -1 → player.move(0, -1) → Z -= 1.0
  Press A: dx = -1 → player.move(-1, 0) → X -= 1.0
  Press D: dx = +1 → player.move(+1, 0) → X += 1.0
  
Result: Grid-based movement (classic Frogger-style)
```

**Step 2: Non-Movement Input (V/C Keys)**
```cpp
if (dx == 0.0f && dz == 0.0f) {
    if (key == 'v' || key == 'V') camera.cyclePreset();
    if (key == 'c' || key == 'C') camera.toggleLock();
    return;
}
```

**Alternative Controls:**
```
V Key: Cycle camera view presets
  Available: Default, Overhead, Side-view, etc.
  Changes visual perspective
  
C Key: Lock/Unlock camera
  Locked: Camera stays on target (locked view)
  Unlocked: Camera can pan freely (or auto-follows)

These keys don't cause movement (early return)
```

**Step 3: Calculate Destination**
```cpp
glm::vec3 currentPos = player.getPosition();
glm::vec3 nextPos = currentPos + glm::vec3(
    dx * Config::CELL_SIZE,
    0.0f,
    dz * Config::CELL_SIZE
);
```

**Destination Calculation:**
```
Current position: [1.0, 0.0, -3.0]
Input: A key (dx=-1, dz=0)

Calculation:
  nextPos.x = 1.0 + (-1 * 1.0) = 0.0
  nextPos.y = 0.0 + 0.0 = 0.0
  nextPos.z = -3.0 + (0 * 1.0) = -3.0
  
Result: nextPos = [0.0, 0.0, -3.0] (one cell left)
```

**Step 4: World Boundary Check**
```cpp
if (std::abs(nextPos.x) >= Config::BOUNDARY_X) return;
if (nextPos.z > Config::BOUNDARY_BACK_Z) return;
```

**Boundary Validation:**
```
BOUNDARY_X = 12 (left/right limits: ±12)
BOUNDARY_BACK_Z = 0 (don't let player retreat past start)

Example rejections:
  nextPos.x = 12.5, |12.5| >= 12? YES → REJECT (too far right)
  nextPos.x = 11.5, |11.5| >= 12? NO → Continue
  nextPos.z = 0.5, > 0? YES → REJECT (going backwards)
  nextPos.z = -5.0, > 0? NO → Continue
  
Purpose: Prevent jumping outside visible play area
```

**Step 5: Find Target Lane**
```cpp
Lane *targetLane = nullptr;
for (auto &lane : lanes) {
    if (std::abs(lane.getZPosition() - nextPos.z) < Config::CELL_SIZE / 2.0f) {
        targetLane = &lane;
        break;
    }
}
```

**Lane Lookup Algorithm:**
```
nextPos.z = -8.5

Searching lanes with Z positions:
  Lane at Z=0: |0 - (-8.5)| = 8.5 > 0.5? YES → Skip
  Lane at Z=-1: |-1 - (-8.5)| = 7.5 > 0.5? YES → Skip
  ...
  Lane at Z=-8: |-8 - (-8.5)| = 0.5 ≥ 0.5? YES → Skip (boundary)
  Lane at Z=-9: |-9 - (-8.5)| = 0.5 ≥ 0.5? YES → Skip (boundary)
  
Wait, should be: Lane at Z=-8: distance = 0.5 which is NOT < 0.5
But lane at Z=-8.5: distance = 0
OR lane at Z=-8: distance = 0.5 (on boundary, 50% tolerance)

Actually checking Z=-8: |-8.0 - (-8.5)| = |-0.5| = 0.5
0.5 < 0.5? NO (just at threshold)

Lane at Z=-8 won't match if exactly 0.5
But typical Z positions are integers: ..., -9, -8, -7, ...
So nextPos.z is usually integer too or X/Y movement

In Z-axis movement (W/S):
  Current Z = -8 (integer)
  Input W: nextPos.z = -8 + (-1*1) = -9
  Find lane at -9: |-9 - (-9)| = 0 < 0.5 ✓ FOUND
```

**Step 6: Decoration/Obstacle Blocking Check (Grass Lanes)**
```cpp
bool blocked = false;
if (targetLane && targetLane->getType() == LANE_GRASS) {
    for (auto &d : targetLane->decorations) {
        float sz = (d.type == 0) ? 0.6f : 0.5f;
        if (std::abs(nextPos.x - d.position.x) < sz &&
            std::abs(nextPos.z - d.position.z) < sz) {
            blocked = true;
            break;
        }
    }
}
```

**Decoration Collision (Grass Only):**
```
Grass lanes may have decorations:
  Type 0: Trees (larger hitbox 0.6)
  Type 1: Flowers (smaller hitbox 0.5)
  
Only check grass lanes (not roads/water)

For each decoration:
  Check if nextPos falls within hitbox

Example:
  Decoration: Tree at [3.0, 0.0, -5.0]
  nextPos: [3.0, 0.0, -5.0]
  Size: 0.6
  
  Distance X: |3.0 - 3.0| = 0.0 < 0.6? YES
  Distance Z: |-5.0 - (-5.0)| = 0.0 < 0.6? YES
  Result: blocked = true → REJECT jump
```

**Step 7: Execute Move (If All Checks Passed)**
```cpp
if (!blocked) player.move(dx, dz);
```

**Move Execution:**
```cpp
// Inside Character::move()
if (!isJumping) {
    basePos.x += dx * Config::CELL_SIZE;
    basePos.z += dz * Config::CELL_SIZE;
    
    startJumpPosition = basePos;
    isJumping = true;
    jumpStartTime = currentTime;
    targetJumpDuration = Config::JUMP_DURATION;
}
```

**Complete Jump Sequence:**
```
User presses W:
  1. onKeyPress() called with 'W'
  2. dz = -1 (forward)
  3. Calculate nextPos = current + (-1, 0, 0) cell forward
  4. Check bounds: within ±12 X? Within start? YES
  5. Find lane at nextPos.z
  6. Lane is ROAD? Skip decoration check
  7. blocked = false, so execute move
  8. player.move(0, -1):
     - isJumping check: false (not jumping)
     - basePos.z -= 1.0 (move 1 cell forward)
     - startJumpPosition = new basePos
     - isJumping = true
     - jumpStartTime = current frame time
  9. Next idle() call:
     - update() called
     - player.update() advances jump animation
     - Character interpolates X/Z linearly toward basePos
     - Character Y follows parabolic arc
  10. After 0.3s:
      - Jump completes
      - isJumping = false
      - next input accepted
```
- Validates boundaries before moving:
  - X boundary: ±13.5 units
  - Z boundary: can't move past starting position (backward death zone)
- Finds target lane for collision with decorations
- Blocks movement if tree/rock in the way on grass lanes
- Only moves if valid

---

# CHARACTER & ANIMATION FILES

## 6. character.h & character.cpp - Player Character

### Character Model Enum

```cpp
enum CharacterModel { MODEL_CHICKEN, MODEL_FROG, MODEL_DINO, MODEL_CAT, MODEL_DOG };
```

**Explanation:** 5 playable character variants

### Death Types

```cpp
enum DeathType { DEATH_NONE, DEATH_SQUISH, DEATH_WATER };
```

**Explanation:** Different death animations - squashed by vehicle or drowning

### Water Particle Structure

```cpp
struct WaterParticle {
    glm::vec3 pos;
    glm::vec3 vel;
    float     size;
    glm::vec3 color;
    float     alpha;
};
```

**Explanation:** Individual water splash particle for water death effect

### Chicken Class Private Members

```cpp
class Chicken {
private:
    glm::vec3 position;
    bool      isJumping;
    glm::vec3 startPos;
    glm::vec3 targetPos;
    float     jumpProgress;
    float     rotationY;
    bool      isDead;
    
    CharacterModel currentModel;
    std::unique_ptr<CharacterBase> modelRenderer;
    
    DeathType                  deathType;
    float                      deathTimer;
    std::vector<WaterParticle> waterParticles;
    float                      waterSurfaceY;
    float                      waterDeathSinkTimer;
    bool                       waterDeathExploded;
```

**Explanation:**
- `position`: Current 3D location
- `isJumping/jumpProgress`: Jump state and interpolation (0-1)
- `startPos/targetPos`: Jump animation endpoints
- `rotationY`: Character facing direction
- `modelRenderer`: Polymorphic renderer for different character types
- Water death particles and sinking timer for drowning animation

### Jump Constants

```cpp
static constexpr float JUMP_DURATION = 0.25f;
static constexpr float JUMP_HEIGHT = 0.75f;
```

**Explanation:** Jump takes 0.25 seconds and reaches 0.75 unit height

### Constructor

```cpp
Chicken::Chicken() {
    setModel(MODEL_CHICKEN);
    reset();
}
```

**Explanation:** Defaults to chicken model, then initializes all state

### Model Selection

```cpp
void Chicken::setModel(CharacterModel model) {
    currentModel = model;
    switch(model) {
        case MODEL_CHICKEN: modelRenderer = std::make_unique<CharacterChicken>(); break;
        case MODEL_FROG:    modelRenderer = std::make_unique<CharacterFrog>();    break;
        case MODEL_DINO:    modelRenderer = std::make_unique<CharacterDino>();    break;
        case MODEL_CAT:     modelRenderer = std::make_unique<CharacterCat>();     break;
        case MODEL_DOG:     modelRenderer = std::make_unique<CharacterDog>();     break;
    }
}
```

**Explanation:** Polymorphic character rendering - each model implements CharacterBase

### Reset State

```cpp
void Chicken::reset() {
    position            = glm::vec3(0.0f, Config::CELL_SIZE * 0.15f, 0.0f);
    isJumping           = false;
    isDead              = false;
    jumpProgress        = 0.0f;
    rotationY           = 180.0f;
    deathType           = DEATH_NONE;
    deathTimer          = 0.0f; 
    waterDeathSinkTimer = 0.0f;
    waterDeathExploded  = false;
    waterParticles.clear();
}
```

**Explanation:** Returns character to starting position and state at spawn

### Water Death Trigger

```cpp
void Chicken::triggerWaterDeath(float surfaceY) {
    isDead              = true;
    deathType           = DEATH_WATER;
    waterSurfaceY       = surfaceY;
    waterDeathSinkTimer = 0.18f;
    waterDeathExploded  = false;
    waterParticles.clear();
}
```

**Detailed Explanation - Water Death State Initialization:**

**Inputs:**
- `surfaceY`: Y-coordinate of water surface (passed by caller in collision check)

**State Transitions:**

```
[ALIVE] → [DEAD_SINKING] → [DEAD_EXPLODED] → [UPDATE_PARTICLES]
           (0.18s)           (spawn 24-32)     (simulate 2-3s)
```

**Step 1: Mark Character as Dead**
```cpp
isDead = true;
deathType = DEATH_WATER;
```
- Prevents further jump input processing
- Signals character rendering to show death model
- Triggers camera to zoom to death location

**Step 2: Record Water Surface Height**
```cpp
waterSurfaceY = surfaceY;
```
- Passed by Game::checkCollisions() when player enters water lane
- Used later in updateWaterParticles() to apply drag when particles submerge
- Example: waterSurfaceY = 0.5f (river surface height)

**Step 3: Initialize Sinking Animation**
```cpp
waterDeathSinkTimer = 0.18f;
waterDeathExploded = false;
```
- `waterDeathSinkTimer`: Countdown from 0.18 seconds
- During this phase, character Y position decreases steadily
- Creates visual effect of sinking into water before explosion
- `waterDeathExploded`: Flag prevents multiple particle spawns

**Step 4: Clear Old Particles**
```cpp
waterParticles.clear();
```
- Prevents accumulation from previous deaths
- Empty vector ready for new particles

### Water Death Particle Spawning

```cpp
void Chicken::spawnWaterDeathParticles() {
    std::vector<glm::vec3> palette = modelRenderer->getWaterDeathPalette();
    int numColors = palette.size();

    int count = 24 + rand() % 8;
    for (int i = 0; i < count; i++) {
        WaterParticle p;
        p.pos = position + glm::vec3(
            ((rand() % 200) - 100) * 0.007f,
            ((rand() % 80))        * 0.006f,
            ((rand() % 200) - 100) * 0.007f
        );

        float angle  = (rand() % 360) * 3.14159265f / 180.0f;
        float hspeed = 0.5f + (rand() % 100) * 0.022f;
        float vspeed = 1.4f + (rand() % 100) * 0.032f;

        if (i < count / 4) {
            vspeed *= 1.9f;
            hspeed *= 0.3f;
        }

        p.vel = glm::vec3(std::cos(angle) * hspeed, vspeed, std::sin(angle) * hspeed);
        p.size  = 0.07f + (rand() % 100) * 0.005f;
        p.color = palette[rand() % numColors];
        p.alpha = 1.0f;
        waterParticles.push_back(p);
    }
}
```

**Detailed Explanation - Particle System Initialization:**

**Algorithm Type**: Radial burst spawning with physics initialization

**Purpose**: Create splashing water effect by spawning particles in all directions

**Step 1: Get Character's Palette Colors**
```cpp
std::vector<glm::vec3> palette = modelRenderer->getWaterDeathPalette();
```
- Different characters have different palettes (Chicken white/orange, Frog green/black, etc.)
- Particles colored from character's specific palette for thematic consistency
- Example Chicken palette: [white, light_orange, orange, pink, red, black]

**Step 2: Determine Particle Count**
```cpp
int count = 24 + rand() % 8;  // Range: 24-31 particles
```
- Base 24 particles + random 0-7 extra
- Creates variation in splash intensity
- Not too many (performance), not too few (effect looks weak)

**Step 3: Position Randomization (Per Particle)**
```cpp
p.pos = position + glm::vec3(
    ((rand() % 200) - 100) * 0.007f,     // X: ±0.7 units
    ((rand() % 80))        * 0.006f,     // Y: 0 to +0.48 units
    ((rand() % 200) - 100) * 0.007f      // Z: ±0.7 units
);
```

**Math Breakdown:**

```
X-axis:
  (rand() % 200) produces 0-199
  - 100 produces -100 to +99
  × 0.007f produces -0.7 to +0.693 (asymmetric, but close)
  
Y-axis:
  (rand() % 80) produces 0-79
  × 0.006f produces 0 to +0.474
  
Z-axis: Same as X-axis
```

**Result**: Particles spawn in sphere around character position:
- Radius ~0.7 units horizontally
- 0-0.47 units vertically (upward burst)

**Why This Distribution:**
- Creates explosion effect centered on character
- Slight upward bias (Y never negative) makes splash look like it's going up
- 3D spread in X/Z makes effect visible from any camera angle

**Step 4: Velocity Initialization (Direction & Speed)**
```cpp
float angle = (rand() % 360) * 3.14159265f / 180.0f;
float hspeed = 0.5f + (rand() % 100) * 0.022f;
float vspeed = 1.4f + (rand() % 100) * 0.032f;
```

**Angle Calculation (Horizontal Direction):**
```
(rand() % 360) produces 0-359 degrees
× π/180 converts to radians (0 to 2π)
Result: Random direction around full circle in XZ plane
```

**Speed Ranges:**

```
Horizontal Speed (hspeed):
  Base: 0.5 units/sec
  Addition: 0-2.2 units/sec (rand % 100 * 0.022)
  Range: 0.5 to 2.7 units/sec

Vertical Speed (vspeed):
  Base: 1.4 units/sec
  Addition: 0-3.2 units/sec (rand % 100 * 0.032)
  Range: 1.4 to 4.6 units/sec
```

**Step 5: Particle Type Variation (First 1/4 Special)**
```cpp
if (i < count / 4) {
    vspeed *= 1.9f;    // Vertical speed up 1.9x
    hspeed *= 0.3f;    // Horizontal speed down to 30%
}
```

**Example (count=28):**
- Particles 0-6: High vertical, narrow horizontal spread
- Particles 7-27: Normal speed in all directions

**Why:** Creates multi-phase splash effect:
- Phase 1: Core particles shoot straight up
- Phase 2: Outer particles spread outward
- Resembles real water splash physics

**Step 6: Velocity Vector Construction**
```cpp
p.vel = glm::vec3(std::cos(angle) * hspeed, vspeed, std::sin(angle) * hspeed);
```

**Math - Converting Polar to Cartesian:**

```
On XZ plane (horizontal):
  x = cos(angle) × hspeed
  z = sin(angle) × hspeed
  
Example with angle=45°=π/4:
  cos(π/4) ≈ 0.707
  sin(π/4) ≈ 0.707
  
  If hspeed = 2.0:
    vx = 0.707 × 2.0 = 1.414
    vz = 0.707 × 2.0 = 1.414
    vy = 2.5 (vspeed)
    
Direction: Northeast, upward
```

**Step 7: Particle Appearance**
```cpp
p.size  = 0.07f + (rand() % 100) * 0.005f;  // 0.07-0.57
p.color = palette[rand() % numColors];
p.alpha = 1.0f;
```
- **Size**: Small variation (0.07 to 0.57 units) so not all same size
- **Color**: Random from character palette (thematic)
- **Alpha**: Fully opaque (1.0) at spawn, will fade during update

**Step 8: Add to Particle List**
```cpp
waterParticles.push_back(p);
```
- Reserves storage for per-frame updates
- Will be simulated during updateWaterParticles()

### Water Particle Update

```cpp
void Chicken::updateWaterParticles(float deltaTime) {
    for (auto& p : waterParticles) {
        if (p.alpha <= 0.0f) continue;

        p.vel.y -= 8.0f * deltaTime;
        p.pos   += p.vel * deltaTime;

        if (p.pos.y < waterSurfaceY) {
            float drag = 1.0f - 6.0f * deltaTime;
            if (drag < 0.0f) drag = 0.0f;
            p.vel.x *= drag;
            p.vel.z *= drag;
            if (p.vel.y > -0.6f) p.vel.y = -0.6f;

            p.alpha -= 2.8f * deltaTime;
            if (p.alpha < 0.0f) p.alpha = 0.0f;
        }
    }
}
```

**Detailed Explanation - Physics Simulation:**

**Algorithm Type**: Particle system with gravity, drag, and submersion

**Per-Frame Simulation Loop:**

**Step 1: Skip Dead Particles**
```cpp
if (p.alpha <= 0.0f) continue;
```
- Alpha reaches 0 when fully transparent
- Skip processing already-invisible particles (performance)

**Step 2: Apply Gravity**
```cpp
p.vel.y -= 8.0f * deltaTime;
```

**Gravity Acceleration:**
```
Downward acceleration: 8.0 units/sec²
(About 81% of Earth's real gravity for better gameplay feel)

Over 1 frame (dt=0.00833s):
  vy_change = -8.0 × 0.00833 = -0.0666 units/sec
  
After 0.5 seconds:
  vy = vspeed - (8.0 × 0.5) = 2.5 - 4.0 = -1.5 units/sec (falling)
```

**Step 3: Update Position**
```cpp
p.pos += p.vel * deltaTime;
```
- Integrate velocity: `pos_new = pos_old + vel × dt`
- Standard Euler integration (simple but sufficient for particles)
- Example: if vel = (1.0, 2.0, 0.5) and dt = 0.016:
  - Change: (0.016, 0.032, 0.008)

**Step 4: Check Water Submersion**
```cpp
if (p.pos.y < waterSurfaceY)
```
- If particle drops below water surface, apply water physics
- Example: waterSurfaceY = 0.5, particle now at Y = 0.4
- Condition true → apply submersion effects

**Step 5: Water Drag (Friction)**
```cpp
float drag = 1.0f - 6.0f * deltaTime;
if (drag < 0.0f) drag = 0.0f;
p.vel.x *= drag;
p.vel.z *= drag;
```

**Drag Coefficient Calculation:**

```
Base formula: drag_factor = 1.0 - 6.0 * dt

Per frame (dt=0.00833s):
  drag_factor = 1.0 - 6.0 * 0.00833 = 1.0 - 0.0498 = 0.95
  Result: Retain 95% of horizontal velocity

After 10 frames (~0.083s in water):
  vel_remaining = vel_initial × (0.95)^10 ≈ vel_initial × 0.60
  
After 30 frames (~0.25s in water):
  vel_remaining = vel_initial × (0.95)^30 ≈ vel_initial × 0.21
```

**Why Clamping to 0:**
```cpp
if (drag < 0.0f) drag = 0.0f;
```
- If dt is very large (frame stutter), drag could go negative
- Negative drag would amplify velocity (wrong!)
- Clamp ensures velocity only decreases

**Why Only X/Z (Not Y):**
```cpp
// Note: p.vel.y NOT multiplied by drag
```
- Vertical drag handled separately (next step)
- Prevents particle from sinking too fast into geometry below water
- Ensures visible "sinking" effect

**Step 6: Vertical Velocity Cap**
```cpp
if (p.vel.y > -0.6f) p.vel.y = -0.6f;
```

**Reasoning:**
```
Without this cap, particle Y velocity could be:
  - Initially: vspeed = 2.5 units/sec (upward)
  - After 0.2s: 2.5 - 8.0*0.2 = 0.9 units/sec (still upward!)
  - After 0.4s: 2.5 - 8.0*0.4 = -0.7 units/sec (now downward)

With cap at -0.6:
  - Once vy = -0.6, it stops accelerating downward
  - Particle sinks at constant -0.6 units/sec
  - Creates controlled, slow sink effect
```

**Effect**: Particles gently sink rather than plummeting through water

**Step 7: Fade Alpha (Transparency)**
```cpp
p.alpha -= 2.8f * deltaTime;
if (p.alpha < 0.0f) p.alpha = 0.0f;
```

**Fade Time Calculation:**

```
Starting alpha: 1.0
Fade rate: 2.8 per second

Time to fully transparent: 1.0 / 2.8 ≈ 0.357 seconds

Fade sequence:
  t=0.00s: alpha = 1.0 (fully opaque)
  t=0.10s: alpha = 1.0 - 2.8*0.10 = 0.72 (72% visible)
  t=0.20s: alpha = 1.0 - 2.8*0.20 = 0.44 (44% visible)
  t=0.36s: alpha ≈ 0.0 (invisible)
```

**Clamp to 0:** Prevents negative alpha (invalid for rendering)

**Complete Particle Lifecycle Timeline (Example Particle):**

```
t=0.000s:  Spawn at player position
           pos = [0.2, 0.5, 0.1]
           vel = [1.5, 2.5, 0.8]
           alpha = 1.0

t=0.016s:  First frame update
           vel.y = 2.5 - 8.0*0.016 = 2.372 (gravity applied)
           pos = [0.224, 0.538, 0.113]
           Still above water (y > 0.5), no drag
           
t=0.100s:  Still airborne
           vel.y ≈ 1.7 (decelerating upward)
           pos ≈ [0.35, 0.65, 0.18]
           alpha = 1.0 - 2.8*0.1 = 0.72 (starting to fade in water)
           
t=0.150s:  Enters water (pos.y = 0.45 < 0.5)
           vel.y = 0 to -1.2 (starts downward)
           Drag applied: vx, vz multiply by ~0.94 each frame
           
t=0.250s:  Underwater
           vel = [reduced horizontal, -0.6]
           pos ≈ [0.3, 0.25, 0.16]
           alpha = 1.0 - 2.8*0.25 = 0.3 (very faded)
           
t=0.360s:  Fully transparent
           alpha < 0, particle skipped in future frames
           Particle effectively removed from rendering
```

**Overall Water Death Effect Result:**
- Character sinks for 0.18s
- Particles burst outward and upward
- Particles arc through air under gravity
- When hitting water, particles slow horizontally and sink gently
- Over ~0.4s, entire splash fades away
- Visual result: Convincing water death with splash effect

### Update Logic

```cpp
void Chicken::update(float deltaTime) {
    if (isDead) {
        deathTimer += deltaTime;
    }

    if (deathType == DEATH_WATER) {
        if (!waterDeathExploded) {
            waterDeathSinkTimer -= deltaTime;
            position.y -= 1.8f * deltaTime;
            if (waterDeathSinkTimer <= 0.0f) {
                waterDeathExploded = true;
                spawnWaterDeathParticles();
            }
        } else {
            updateWaterParticles(deltaTime);
        }
        return;
    }

    if (!isJumping) return;

    jumpProgress += deltaTime / JUMP_DURATION;

    if (jumpProgress >= 1.0f) {
        position    = targetPos;
        isJumping   = false;
        jumpProgress = 0.0f;
    } else {
        position.x = startPos.x + (targetPos.x - startPos.x) * jumpProgress;
        position.z = startPos.z + (targetPos.z - startPos.z) * jumpProgress;
        float parabola = 4.0f * JUMP_HEIGHT * jumpProgress * (1.0f - jumpProgress);
        position.y = startPos.y + parabola;
    }
}
```

**Detailed Explanation - Character Update State Machine:**

**State Hierarchy:**

```
    ┌─ ALIVE
    │  └─ isJumping? YES → Jump update
    │                NO  → No update
    │
    └─ DEAD
       ├─ DEATH_WATER
       │  ├─ !waterDeathExploded → Sink phase (0.18s)
       │  └─  waterDeathExploded → Particle simulation
       │
       └─ DEATH_SQUISH → (other death handling)
```

**Step 1: Increment Death Timer (All Death States)**
```cpp
if (isDead) {
    deathTimer += deltaTime;
}
```
- Tracks total time character has been dead
- Used to decide when to transition to game over state
- Counts independently from water death timer

**Step 2: Check Water Death**
```cpp
if (deathType == DEATH_WATER)
```
- If currently in water death sequence, handle specially
- Overrides jump logic

**Step 3a: Sinking Phase**
```cpp
if (!waterDeathExploded) {
    waterDeathSinkTimer -= deltaTime;
    position.y -= 1.8f * deltaTime;
    if (waterDeathSinkTimer <= 0.0f) {
        waterDeathExploded = true;
        spawnWaterDeathParticles();
    }
}
```

**Timeline:**
```
Initial: waterDeathSinkTimer = 0.18s, position.y = 0.5

t=0.050s: timer = 0.13s, y -= 0.09, y ≈ 0.41
t=0.100s: timer = 0.08s, y -= 0.18, y ≈ 0.32
t=0.150s: timer = 0.03s, y -= 0.27, y ≈ 0.23
t=0.180s: timer = 0.00s → EXPLODE!
          spawnWaterDeathParticles() called
          waterDeathExploded = true
```

**Sinking Speed: 1.8 units/sec**
- Character descends while particles spawn
- Creates lag effect (character sinking while explosion happens)

**Step 3b: Particle Simulation Phase**
```cpp
} else {
    updateWaterParticles(deltaTime);
}
return;  // Exit - skip jump logic
```
- Once exploded, update all particles every frame
- `return` prevents jump logic from running

**Step 4: Early Exit for Non-Jumping**
```cpp
if (!isJumping) return;
```
- If alive and not currently jumping, nothing to update
- Waits for next jump command

**Step 5: Jump Progress Integration**
```cpp
jumpProgress += deltaTime / JUMP_DURATION;
```

**Calculation:**
```
JUMP_DURATION = 0.25 seconds

Per frame (dt = 0.00833s):
  jumpProgress += 0.00833 / 0.25 = 0.0333
  
After ~7-8 frames:
  jumpProgress ≈ 0.25 = 1.0 (jump complete)
```

**Step 6: Jump Completion Check**
```cpp
if (jumpProgress >= 1.0f) {
    position = targetPos;
    isJumping = false;
    jumpProgress = 0.0f;
}
```
- When progress reaches 100%, snap to target
- Reset flags for next jump
- Ensures exact final position (no floating-point drift)

**Step 7: Jump Animation (Linear & Parabolic)**
```cpp
position.x = startPos.x + (targetPos.x - startPos.x) * jumpProgress;
position.z = startPos.z + (targetPos.z - startPos.z) * jumpProgress;
float parabola = 4.0f * JUMP_HEIGHT * jumpProgress * (1.0f - jumpProgress);
position.y = startPos.y + parabola;
```

**X/Z Interpolation (Linear):**
```
Example: startPos.x = 0, targetPos.x = 2, jumpProgress = 0.5
position.x = 0 + (2 - 0) * 0.5 = 1.0

At progress=0:     x = start (0)
At progress=0.5:   x = start + 0.5*(target-start) = midpoint
At progress=1.0:   x = target (2)

Result: Linear motion horizontally
```

**Y Arc (Parabolic):**
```
Formula: y = start + 4 * JUMP_HEIGHT * t * (1 - t)

JUMP_HEIGHT = 0.75 units
At progress=0:      y = 0 + 4 * 0.75 * 0 * 1 = 0
At progress=0.25:   y = 0 + 4 * 0.75 * 0.25 * 0.75 = 0.5625 (peak!)
At progress=0.5:    y = 0 + 4 * 0.75 * 0.5 * 0.5 = 0.75
At progress=0.75:   y = 0 + 4 * 0.75 * 0.75 * 0.25 = 0.5625
At progress=1.0:    y = 0 + 4 * 0.75 * 1 * 0 = 0

Peak at t=0.5: y_max = 0.75 units
```

**Why This Formula:**
```
f(t) = 4ht(1-t) creates symmetric parabola
  - t=0: f(0) = 0 (start)
  - t=0.5: f(0.5) = 4h*0.5*0.5 = h (peak = JUMP_HEIGHT)
  - t=1: f(1) = 0 (end)
  
Multiplying by 4 ensures max height equals JUMP_HEIGHT
Without the 4, max would be h/4
```

**Result**: Character follows realistic arc trajectory during jump
- When complete, snaps to exact target position

### Movement

```cpp
void Chicken::move(float gridX, float gridZ) {
    if (isJumping) return;

    startPos   = position;
    targetPos  = position + glm::vec3(gridX, 0.0f, gridZ);
    isJumping  = true;
    jumpProgress = 0.0f;
    
    if (gridX < 0) rotationY = 90.0f;
    if (gridX > 0) rotationY = 270.0f;
    if (gridZ < 0) rotationY = 180.0f;
    if (gridZ > 0) rotationY = 0.0f;
}
```

**Explanation:**
- Ignores move commands while already jumping (single-jump action)
- Sets jump start/end positions
- Rotates character to face direction (90/180/270/0 for left/back/right/forward)

### Log Velocity

```cpp
void Chicken::applyLogVelocity(float velocityX, float deltaTime) {
    position.x += velocityX * deltaTime;
}
```

**Explanation:** When standing on log, applies log's X velocity to character each frame

---

## 7. character_chicken.cpp - Chicken Model Implementation

### Water Death Palette

```cpp
std::vector<glm::vec3> CharacterChicken::getWaterDeathPalette() const {
    return {{1,1,1},{1,1,1},{1,1,1},{1,.5f,.05f},{1,.5f,.05f},
            {1,.20f,.55f},{.85f,.10f,.10f},{.05f,.05f,.05f}};
}
```

**Explanation:** Colors for water death particles (mostly white, some orange, pink, red, black)

### Model Drawing

```cpp
void CharacterChicken::drawModel(Renderer& renderer) const {
    glm::vec3 wht={1,1,1},    owt={.92f,.92f,.88f};
    glm::vec3 org={1,.50f,.05f}, ord={.85f,.40f,.02f};
    glm::vec3 red={.85f,.10f,.10f}, pnk={1,.20f,.55f};
    glm::vec3 blk={.05f,.05f,.05f}, yel={.95f,.85f,.10f};

    // Body (layered for plump egg shape)
    renderer.drawCube({0,.50f,0},  {.68f,.28f,.68f},wht);
    renderer.drawCube({0,.64f,0},  {.60f,.16f,.60f},wht);
    renderer.drawCube({0,.34f,.02f},{.62f,.14f,.62f},owt); // belly
    renderer.drawCube({0,.50f,.28f},{.44f,.24f,.22f},wht); // breast bulge
```

**Explanation:** Builds chicken from colored cubes:
- Base body: white oval shape (plump)
- Belly: off-white slightly forward
- Breast: bulge on front
- Each cube positioned in 3D space and scaled for proper shape

### Wings, Tail, Neck

```cpp
    // Wings: 3-layer stubs, symmetric
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.36f,.56f,.04f},{.14f,.24f,.46f},owt);
        renderer.drawCube({s*.42f,.50f,0},   {.08f,.18f,.36f},wht);
        renderer.drawCube({s*.44f,.44f,-.04f},{.06f,.12f,.28f},owt);
    }

    // Tail feathers: base + centre plume + 2 side plumes
    renderer.drawCube({0,.64f,-.38f},{.30f,.14f,.12f},wht);
    renderer.drawCube({0,.78f,-.42f},{.08f,.28f,.08f},wht);
    for(float s:{-1.f,1.f})
        renderer.drawCube({s*.14f,.74f,-.40f},{.08f,.24f,.07f},owt);
```

**Explanation:**
- Wings: 3 stacked layers per side for feathered effect
- Tail: Base rectangle + tall center plume + two side plumes for fluffy tail

### Head & Facial Features

```cpp
    float y_offset_head = -0.18f;
    // Head + puffed cheeks
    renderer.drawCube({0,1.02f + y_offset_head,.28f},{.44f,.40f,.42f},wht);
    for(float s:{-1.f,1.f})
        renderer.drawCube({s*.24f,.98f + y_offset_head,.34f},{.10f,.14f,.18f},owt);

    // Comb: base strip + centre lobe (tallest) + 2 side lobes
    renderer.drawCube({0,1.22f + y_offset_head,.22f},{.22f,.06f,.10f},red);
    renderer.drawCube({0,1.28f + y_offset_head,.24f},{.08f,.12f,.10f},red);
    for(float s:{-1.f,1.f})
        renderer.drawCube({s*.10f,1.24f + y_offset_head,.22f},{.07f,.08f,.08f},red);

    // Wattle
    renderer.drawCube({0,.88f + y_offset_head,.50f},{.13f,.16f,.08f},pnk);
    renderer.drawCube({0,.80f + y_offset_head,.50f},{.10f,.10f,.07f},pnk);

    // Beak (upper + lower)
    renderer.drawCube({0,1.00f + y_offset_head,.54f},{.14f,.07f,.14f},org);
    renderer.drawCube({0,.94f + y_offset_head,.52f}, {.12f,.06f,.12f},ord);

    // Eyes: yellow iris, pupil, shine – symmetric
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.20f,1.06f + y_offset_head,.46f},{.10f,.10f,.06f},yel);
        renderer.drawCube({s*.20f,1.06f + y_offset_head,.50f},{.06f,.06f,.04f},blk);
        renderer.drawCube({s*.20f+.04f,1.10f + y_offset_head,.52f},{.03f,.03f,.02f},wht);
    }
```

**Explanation:**
- Head: Main sphere + puffy cheeks
- Red comb: Three-lobe crown on top
- Pink wattle: Chin flaps
- Orange beak: Upper and lower parts
- Eyes: Yellow + black pupil + white highlight per side

### Legs & Feet

```cpp
    static const struct{float t,z,sz;}toe[]={{-.08f,.22f,.12f},{0.f,.26f,.14f},{.08f,.22f,.12f}};
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.20f,.28f,.04f},{.12f,.18f,.12f},org);
        renderer.drawCube({s*.20f,.12f,.04f},{.10f,.14f,.10f},ord);
        renderer.drawCube({s*.20f,.04f,.10f},{.10f,.06f,.18f},org);
        for(auto& tk:toe)
            renderer.drawCube({s*(.20f+tk.t),.03f,tk.z},{.05f,.04f,tk.sz},ord);
        renderer.drawCube({s*.20f,.03f,-.06f},{.05f,.04f,.10f},ord); // back spur
    }
}
```

**Explanation:**
- Per leg (left/right):
  - Upper leg (thigh)
  - Lower leg (shin)
  - Foot pad
- Three toes forward (offset left, center, right)
- One toe back (spur)

---

# GAMEPLAY MECHANICS

## 8. lane.h & lane.cpp - Lane Generation & Management

### Lane Structure

```cpp
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
        glm::vec3 position;
    };
    std::vector<SignalPost> signalPosts;
    int safePathColumn;
```

**Explanation:** Complete lane data:
- `zPosition`: Where lane sits in Z space
- `obstacles`: Cars, trains, logs, lilypads
- `coins`: Collectibles
- `decorations`: Trees and rocks on grass
- `signalPosts`: Visual markers on train tracks
- `safePathColumn`: Where guaranteed safe passage is

### Lane Constructor

```cpp
Lane::Lane(float z, LaneType t, int safePath)
    : zPosition(z), type(t), safePathColumn(safePath) {

    float dir = (rand() % 2 == 0) ? 1.0f : -1.0f;

    // ===== EXISTING OBSTACLES =====
    if (type == LANE_ROAD) {
        float speed = dir * randomRange(Config::CAR_SPEED_MIN, Config::CAR_SPEED_MAX);
        float startX = dir * -(5.0f + static_cast<float>(rand() % 10));
        
        int pick = rand() % 3;
        VehicleVariant variant = (pick == 1) ? VEHICLE_BIG_CAR
                                : (pick == 2) ? VEHICLE_TRUCK
                                : VEHICLE_SMALL_CAR;

        obstacles.push_back(Obstacle(
            glm::vec3(startX, 0.31f, zPosition),
            speed, OBSTACLE_CAR, variant));
    }
```

**Explanation - Road Lanes:**
- Random direction (left/right)
- Speed between 2-4 units/sec in that direction
- Starts off-screen (±5-15 units)
- Random vehicle type (33% each of small/big/truck)

### Rail Lanes

```cpp
    else if (type == LANE_RAIL) {
        float speed  = dir * Config::TRAIN_SPEED;
        float startX = dir * -(35.0f + static_cast<float>(rand() % 20));

        obstacles.push_back(Obstacle(
            glm::vec3(startX, 0.46f, zPosition),
            speed, OBSTACLE_TRAIN));

        // ── Signal posts every SIGNAL_SPACING units across the track ─────────

        for (float sx = -Config::SIGNAL_RANGE; sx <= Config::SIGNAL_RANGE + 0.01f; sx += Config::SIGNAL_SPACING) {
            float jitter = Config::SIGNAL_JITTER * (randomRange(0.0f, 1.0f) * 2.0f - 1.0f);
            float finalX = sx + jitter;
            if (finalX < -Config::SIGNAL_RANGE) finalX = -Config::SIGNAL_RANGE;
            if (finalX >  Config::SIGNAL_RANGE) finalX =  Config::SIGNAL_RANGE;

            float safeWorldX = safePathColumn * Config::CELL_SIZE;
            if (std::abs(finalX - safeWorldX) < Config::CELL_SIZE * 0.7f)
                continue;

            SignalPost sp;
            sp.position = glm::vec3(finalX, 0.0f, zPosition + Config::SIGNAL_Z_SIDE);
            signalPosts.push_back(sp);
        }
    }
```

**Explanation:**
- Single train per rail lane, starts further off-screen (±35-55 units)
- Signal posts every 8 units along track with ±1.4 random jitter
- Posts don't spawn on safe path column (±0.7 avoidance)
- Posts offset slightly in Z for visual side placement

### River Lanes with Log Sets

```cpp
    else if (type == LANE_RIVER) {
        float speed = dir * randomRange(Config::LOG_SPEED_MIN, Config::LOG_SPEED_MAX);
        
        float startSetX = -((Config::LOG_SETS - 1) * Config::LOG_SET_GAP) / 2.0f;
        
        for (int s = 0; s < Config::LOG_SETS; s++) {
            int logCount = Config::LOG_COUNT_MIN +
                rand() % (Config::LOG_COUNT_MAX - Config::LOG_COUNT_MIN + 1);
        
            float totalSpan = (logCount - 1) * Config::LOG_SPACING;
            float setCenterX = startSetX + (s * Config::LOG_SET_GAP);
            float startX = setCenterX - (totalSpan / 2.0f);
        
            for (int i = 0; i < logCount; i++) {
                float x = startX + i * Config::LOG_SPACING;
            
                obstacles.push_back(Obstacle(
                    glm::vec3(x, Config::LOG_Y, zPosition),
                    speed, OBSTACLE_LOG));
            }
        }
    }
```

**Explanation:**
- Two sets of logs per lane with 15 units gap
- Each set has 2-3 logs with 4.0 unit spacing
- Sets centered around calculated positions
- All logs move at same speed (0.75-1.5 units/sec)

### Lilypad Lanes

```cpp
    else if (type == LANE_LILYPAD) {
        // 1. Guaranteed lilypad on the safe path
        float safeX = safePathColumn * Config::CELL_SIZE;
        obstacles.push_back(Obstacle(
            glm::vec3(safeX, Config::LILYPAD_Y, zPosition), 
            0.0f, OBSTACLE_LILYPAD));

        // 2. Randomly spawn lilypads to the LEFT
        float currX = safeX - randomRange(Config::LILYPAD_GAP_MIN, Config::LILYPAD_GAP_MAX);
        while (currX > -15.0f) {
            obstacles.push_back(Obstacle(
                glm::vec3(currX, Config::LILYPAD_Y, zPosition), 
                0.0f, OBSTACLE_LILYPAD));
            currX -= randomRange(Config::LILYPAD_GAP_MIN, Config::LILYPAD_GAP_MAX);
        }

        // 3. Randomly spawn lilypads to the RIGHT
        currX = safeX + randomRange(Config::LILYPAD_GAP_MIN, Config::LILYPAD_GAP_MAX);
        while (currX < 15.0f) {
            obstacles.push_back(Obstacle(
                glm::vec3(currX, Config::LILYPAD_Y, zPosition), 
                0.0f, OBSTACLE_LILYPAD));
            currX += randomRange(Config::LILYPAD_GAP_MIN, Config::LILYPAD_GAP_MAX);
            }
        }
```

**Explanation:**
- One guaranteed lilypad at safe path column (ensures crossing is possible)
- Random gaps (1.2-10 units) between lilypads on each side
- Fills left side from safe point to boundary (-15)
- Fills right side from safe point to boundary (15)

### Grass Decorations

```cpp
    if (type == LANE_GRASS) {
        int pathWidth = 2;
        int pathBufferZone = pathWidth + 1;

        for (int i = -15; i <= 15; i++) {
            float x = i * Config::CELL_SIZE * 0.9f;

            if (std::abs(i - safePathColumn) <= pathBufferZone)
                continue;

            if (std::abs(i - safePathColumn) <= pathBufferZone + 2) {
                if (rand() % 100 > 30) continue;
            } else {
                if (rand() % 100 > 75) continue;
            }

            if (!isFarEnough(decorations, x))
                continue;

            Decoration d;
            d.position = glm::vec3(x, 0.15f, zPosition);

            if (rand() % 5 == 0) {
                d.type = 1;
                d.scale = 0.8f;
                d.color = glm::vec3(0.5f);
            } else {
                d.type = 0;
                d.scale = (rand() % 2 == 0) ? 1.0f : 1.5f;
                float greenVar = 0.1f * (rand() % 3);
                d.color = glm::vec3(0.2f, 0.7f + greenVar, 0.2f);
            }

            decorations.push_back(d);
        }
    }
```

**Explanation:**
- Spans ±15 cells width
- Avoids safe path ±3 column buffer
- Density near path: 30% spawn chance
- Density far from path: 75% spawn chance
- 20% rocks, 80% trees
- Trees: random green shade, random 1.0-1.5x scale
- Rocks: gray, 0.8x scale
- Avoids duplicates in same spot

### Coin Spawning

```cpp
    if (type == LANE_GRASS&&(rand()%100)<35) {
        int tries = 5;

        while (tries--) {
            int offset = rand() % 3 - 1;
            int coinCol = safePathColumn + offset;
            float x = coinCol * Config::CELL_SIZE * 0.9f;

            bool overlap = false;
            for (auto& d : decorations) {
                if (fabs(d.position.x - x) < Config::CELL_SIZE * 0.8f) {
                    overlap = true;
                    break;
                }
            }

            if (!overlap) {
                coins.emplace_back(glm::vec3(x, Config::CELL_SIZE * 0.6f, zPosition));
                break;
            }
        }
    }
```

**Explanation:**
- 35% chance to spawn coin on grass lanes
- Tries 5 times to find non-overlapping spot near safe path
- Avoids decorations (trees/rocks) within 0.8 unit distance
- Heights above ground at 0.6 unit visibility level

---

## 9. obstacle.cpp - Obstacle Physics & Rendering

### Obstacle Constructor

```cpp
Obstacle::Obstacle(glm::vec3 startPos, float spd, ObstacleType t, VehicleVariant variant)
    : position(startPos), speed(spd), type(t), vehicleVariant(variant),
      isActive(true), sinkOffset(0.0f), isSinking(false),
      startPosition(startPos), respawnTimer(0.0f), fastStream(false)
{
    if (type == OBSTACLE_CAR) {
        if      (vehicleVariant == VEHICLE_SMALL_CAR) size = glm::vec3(1.45f, 0.62f, 0.78f);
        else if (vehicleVariant == VEHICLE_BIG_CAR)   size = glm::vec3(1.95f, 0.72f, 0.88f);
        else                                           size = glm::vec3(3.05f, 0.92f, 1.00f);
    }
    else if (type == OBSTACLE_TRAIN) {
        size = glm::vec3(Config::TRAIN_LENGTH * Config::CELL_SIZE,
                         Config::CELL_SIZE * 1.0f,
                         Config::CELL_SIZE * 0.9f);
        respawnDelay = 5.0f + static_cast<float>(rand() % 6);
    }
    else if (type == OBSTACLE_LOG) {
        size = glm::vec3(Config::LOG_WIDTH, Config::LOG_HEIGHT, Config::LOG_DEPTH);
    }
    else if (type == OBSTACLE_LILYPAD) {
        size = glm::vec3(Config::LILYPAD_SIZE, Config::LILYPAD_HEIGHT, Config::LILYPAD_SIZE);
    }
}
```

**Explanation:** Sets up obstacle type with proper dimensions:
- Small car: 1.45×0.62×0.78
- Big car: 1.95×0.72×0.88
- Truck: 3.05×0.92×1.00
- Train: ~15×1×0.9 (depends on TRAIN_LENGTH)
- Log: 2.8×0.28×0.75
- Lilypad: 0.9×0.15×0.9

### Obstacle Update - Train

```cpp
void Obstacle::update(float deltaTime) {
    if (type == OBSTACLE_TRAIN) {
        if (isActive) {
            position.x += speed * deltaTime;
            if ((speed > 0 && position.x > 65.0f) || (speed < 0 && position.x < -65.0f)) {
                isActive     = false;
                respawnTimer = respawnDelay;
            }
        } else {
            respawnTimer -= deltaTime;
            if (respawnTimer <= 0.0f) {
                position = startPosition;
                isActive = true;
            }
        }
        return;
    }
```

**Detailed Explanation - Train Despawn/Respawn Cycle:**

**Algorithm Type**: Active/inactive state machine with timer-based cycling

**Inputs:**
- `speed`: Train velocity (±25 units/sec)
- `position.x`: Current horizontal position
- `respawnDelay`: Random 5-10 seconds

**Step 1: Check If Train Still Active**
```cpp
if (isActive) {
    position.x += speed * deltaTime;
```
- If active, move train at constant velocity
- `position.x += speed * deltaTime` standard physics integration

**Step 2: Check Despawn Condition**
```cpp
if ((speed > 0 && position.x > 65.0f) || (speed < 0 && position.x < -65.0f)) {
```

**Condition Analysis:**

```
Case 1: speed > 0 (moving right)
  Check: position.x > 65.0f?
  If true: Train moved off right edge, despawn
  
Case 2: speed < 0 (moving left)
  Check: position.x < -65.0f?
  If true: Train moved off left edge, despawn
  
Case 3: stationary or slow
  Conditions never true, train never despawns
```

**Why ±65?**
```
Camera can see roughly ±12 to ±15 units horizontally
Game boundary is technically ±60
±65 gives margin beyond visual boundary
At 25 units/sec, train offscreen for (65-15)/25 ≈ 2 seconds
Prevents train popping into view unexpectedly
```

**Step 3: Trigger Despawn**
```cpp
    isActive = false;
    respawnTimer = respawnDelay;
```
- Set to inactive (skip rendering, collision)
- Initialize respawn countdown timer
- `respawnDelay` = 5-10 seconds (random, set when train created)

**Step 4: Inactive State - Countdown**
```cpp
} else {
    respawnTimer -= deltaTime;
```
- Train not rendering or colliding
- Timer decrements each frame
- After 5-10 seconds, respawnTimer reaches 0

**Step 5: Respawn**
```cpp
    if (respawnTimer <= 0.0f) {
        position = startPosition;
        isActive = true;
    }
```
- Reset position to start (typically [0, y, z] for most train lanes)
- Set active = true (resume rendering, collision)
- Train emerges from starting position

**Timeline Example (20 FPS simulation for clarity):**

```
t=0.0s:   Train active, x=0, speed=25 (moving right)
t=0.5s:   x += 25*0.05 = 1.25, x=1.25
t=1.0s:   x=2.5
t=2.0s:   x=5.0
t=2.6s:   x ≈ 6.5, DESPAWN TRIGGER!
          isActive=false, respawnTimer=7.3s (example)
t=2.6-9.9s: Train invisible, not colliding
t=9.9s:   respawnTimer≈0, RESPAWN!
          position=[0, y, z], isActive=true
t=10.4s:  x=1.25 (train restarted)

Total cycle time: ~10.3 seconds
```

### Log Physics - Fast-Stream

```cpp
    if (type == OBSTACLE_LOG) {
        if (fastStream) {
            float sign     = (speed >= 0.0f) ? 1.0f : -1.0f;
            float absSpeed = std::abs(speed);
            absSpeed += absSpeed * Config::LOG_STREAM_ACCEL * deltaTime;
            absSpeed  = std::min(absSpeed, Config::LOG_STREAM_MAX_SPEED);
            speed     = sign * absSpeed;

            position.x += speed * deltaTime;

            if (std::abs(position.x) > Config::LOG_STREAM_EXIT_X) {
                isActive    = false;
                fastStream  = false;
                position.x  = (speed > 0) ? -15.0f : 15.0f;
                speed = (speed > 0 ? 1.0f : -1.0f)
                        * (Config::LOG_SPEED_MIN +
                           static_cast<float>(rand()) /
                           static_cast<float>(RAND_MAX) *
                           (Config::LOG_SPEED_MAX - Config::LOG_SPEED_MIN));
                isActive = true;
            }
        } else {
```

**Detailed Explanation - Exponential Acceleration Physics:**

**Algorithm Type**: Exponential velocity growth with max cap and state transition

**Purpose**: Create acceleration zone where logs become dangerous when at foam boundary

**Configuration Parameters:**
- `Config::LOG_STREAM_ACCEL`: 6.0 (acceleration multiplier)
- `Config::LOG_STREAM_MAX_SPEED`: 22.0 units/sec
- `Config::LOG_STREAM_EXIT_X`: ±20 units
- Normal log speed: 3-8 units/sec

**Step 1: Preserve Direction**
```cpp
float sign = (speed >= 0.0f) ? 1.0f : -1.0f;
float absSpeed = std::abs(speed);
```

**Purpose:** Separate speed magnitude from direction
- Extract sign: +1 if moving right, -1 if moving left
- Get absolute speed (always positive)
- Allows acceleration calculation in positive domain only

**Example:**
```
speed = -5.0 (moving left at 5 units/sec)
sign = -1.0
absSpeed = 5.0
```

**Step 2: Apply Exponential Acceleration**
```cpp
absSpeed += absSpeed * Config::LOG_STREAM_ACCEL * deltaTime;
```

**Math Expansion:**
```
absSpeed_new = absSpeed + absSpeed * accel * dt
             = absSpeed * (1 + accel * dt)
             
With accel=6.0, dt=0.00833s:
  factor = 1 + 6.0 * 0.00833 = 1.05
  absSpeed_new = absSpeed_old * 1.05
  
Result: Every frame, speed increases by 5%
```

**Exponential Growth Pattern:**
```
Frame 1: speed = 5.0
Frame 2: speed = 5.0 * 1.05 = 5.25
Frame 3: speed = 5.25 * 1.05 = 5.51
Frame 4: speed = 5.51 * 1.05 = 5.79
Frame 5: speed = 5.79 * 1.05 = 6.08
...
Frame 30: speed ≈ 5.0 * (1.05)^29 ≈ 22.0 (maxed out)

Time: ~30 frames × 0.00833s ≈ 0.25 seconds to reach max speed
```

**Why Exponential vs Linear:**
```
Linear (accel = 3.0/sec):
  speed_new = speed_old + 3.0 * dt
  Frame 30: speed = 5.0 + 3.0*0.25 = 5.75 (barely accelerates)

Exponential (accel = 6.0):
  Frame 30: speed = 22.0 (runaway!)
  
Exponential creates runaway effect (hard to control)
Linear would be too gentle
Exponential is what player experiences when caught at foam boundary
```

**Step 3: Cap Maximum Speed**
```cpp
absSpeed = std::min(absSpeed, Config::LOG_STREAM_MAX_SPEED);
```

**Purpose:** Prevent speed from growing infinitely
- Cap at 22 units/sec
- Without this cap, speed could reach hundreds after long acceleration
- Cap creates "terminal velocity" effect

**Example:**
```
Before cap: absSpeed = 23.5 (exceeded 22.0 limit)
After cap:  absSpeed = 22.0 (limited)

At limit, next frame:
  absSpeed_new = 22.0 * 1.05 = 23.1
  cap: absSpeed_new = 22.0
  
Result: Speed maintains at max (stops growing)
```

**Step 4: Reapply Direction**
```cpp
speed = sign * absSpeed;
```
- Multiply magnitude by sign
- Reconstructs signed velocity

**Example:**
```
If moving left:
  sign = -1.0, absSpeed = 22.0
  speed = -1.0 * 22.0 = -22.0

Result: Moving left at 22 units/sec
```

**Step 5: Update Position**
```cpp
position.x += speed * deltaTime;
```
- Standard physics integration
- At max speed (22 units/sec), moves 0.183 units per frame
- Log rapidly crosses screen

**Step 6: Check Stream Exit Condition**
```cpp
if (std::abs(position.x) > Config::LOG_STREAM_EXIT_X)
```
- `std::abs()` gives absolute value
- Exit at ±20 units (beyond normal wrap at ±15)
- Reason: Need more distance for high-speed exit

**Condition:**
```
Normal wraparound: ±15 units
Fast stream exit: ±20 units

Rationale:
  Normal log at 8 units/sec reaches ±15 in ~2 seconds
  Fast log at 22 units/sec reaches ±20 in ~1 second
  Gives distinct behavior for each state
```

**Step 7: Transition Back to Normal State**
```cpp
        isActive = false;
        fastStream = false;
        position.x = (speed > 0) ? -15.0f : 15.0f;
```

**State Transition Logic:**

```
Before exit:
  isActive = true (rendering, colliding)
  fastStream = true (accelerating)
  
After exit:
  isActive = false (invisible, no collision)
  fastStream = false (stop accelerating)
  position = respawn point:
    If exiting right (speed > 0): reappear at x = -15.0 (left edge)
    If exiting left (speed < 0): reappear at x = +15.0 (right edge)
```

**Why Reappear Opposite Side:**
```
Example: Log exits right (x > 20)
  Set position = -15 (reappear on left)
  Result: Log comes back as if cycling through
  If it entered stream going right, exits right, reappears left
  Creates continuous loop illusion
```

**Step 8: Randomize New Speed**
```cpp
speed = (speed > 0 ? 1.0f : -1.0f)
        * (Config::LOG_SPEED_MIN +
           static_cast<float>(rand()) /
           static_cast<float>(RAND_MAX) *
           (Config::LOG_SPEED_MAX - Config::LOG_SPEED_MIN));
```

**Speed Randomization Formula:**
```
rand() / RAND_MAX = random float in [0, 1)
speed_range = LOG_SPEED_MAX - LOG_SPEED_MIN = 8 - 3 = 5
random_addition = 0 to 5 * random = 0 to 5
final_speed = 3 + {0 to 5} = 3 to 8 units/sec

Preserves direction:
  (speed > 0 ? 1.0f : -1.0f) multiplies result by ±1
```

**Example:**
```
Exited stream going right (speed > 0)
rand() returns 0.5
final_speed = 1.0 * (3 + 5*0.5) = 5.5 units/sec (new random speed)

Next log cycle:
  Normal movement at 5.5 units/sec
  When crossing foam boundary (+12 to -12), triggers fast stream again
```

**Step 9: Reactivate**
```cpp
        isActive = true;
```
- Log now visible and colliding again
- Back to normal movement (non-accelerating)
- Cycle ready to repeat

**Complete Fast-Stream Sequence Example:**

```
t=0.0s:   Log at x=12.5, speed=4.0, normal state
t=0.05s:  x=12.7, enters foam boundary (±12) → fastStream triggers!
          absSpeed=4.0, enter acceleration phase
          
t=0.10s:  absSpeed = 4.0 * 1.05 = 4.2
t=0.20s:  absSpeed = 4.0 * (1.05)^2 ≈ 4.41
t=0.50s:  absSpeed ≈ 4.0 * (1.05)^10 ≈ 6.5
t=1.00s:  absSpeed ≈ 4.0 * (1.05)^20 ≈ 10.6
t=1.50s:  absSpeed ≈ 4.0 * (1.05)^30 ≈ 17.3
t=1.75s:  absSpeed ≈ 4.0 * (1.05)^35 ≈ 21.3
t=1.85s:  absSpeed = 22.0 (capped at max)

Movement during acceleration:
  Distance = sum of (speed_i * dt) for all frames
           ≈ 4.0*0.05 + 4.2*0.05 + ... + 22.0*0.05
           ≈ 9.5 units (from x=12.5 to x=22)

t=1.85s:  x≈22.0, exits fast stream (abs(x) > 20)
          isActive = false
          position.x = -15.0 (reappear on left, since moving right)
          speed = ±5.2 (new random 3-8 range)
          fastStream = false
          isActive = true
          
t=1.90s:  Log now at x=-15.0, moving at 5.2 units/sec normally
          Back to regular behavior until foam boundary reached again
```

**Real-World Impact for Player:**

```
Scenario: Player rides a log at x=12 (foam boundary)
  
Normal log behavior:
  Log speed ≈ 4 units/sec
  Player slowly carried left-right
  
Fast stream scenario:
  Log speed accelerates from 4 → 22 units/sec over ~1.5 seconds
  Player loses control, carried rapidly off screen
  If player doesn't jump off, game over!
  
Strategic element:
  Player must recognize foam boundary
  Jump off before acceleration becomes uncontrollable
  OR time jump carefully to avoid drowning
```

### Normal Log Physics

```cpp
            position.x += speed * deltaTime;
            if (speed > 0 && position.x >  15.0f) position.x = -15.0f;
            if (speed < 0 && position.x < -15.0f) position.x =  15.0f;
        }

        // Sink offset lerp
        float target  = isSinking ? -Config::LOG_SINK_AMOUNT : 0.0f;
        sinkOffset   += (target - sinkOffset) * Config::LOG_SINK_SPEED * deltaTime;
        isSinking     = false;
        return;
    }
```

**Detailed Explanation - Normal Log Movement & Sinking Animation:**

**Step 1: Normal Wraparound**
```cpp
position.x += speed * deltaTime;
if (speed > 0 && position.x >  15.0f) position.x = -15.0f;
if (speed < 0 && position.x < -15.0f) position.x =  15.0f;
```

**Boundary Wrapping Logic:**

```
Moving right (speed > 0):
  If position.x > 15.0 (exited right edge):
    Teleport to x = -15.0 (reappear on left)
    Creates infinite loop appearance

Moving left (speed < 0):
  If position.x < -15.0 (exited left edge):
    Teleport to x = +15.0 (reappear on right)
    
Result: Seamless wraparound at screen boundaries
        Like scrolling background wrapping
```

**Why ±15 (Not ±12 Or ±20):**
```
Exact width of playable area
±15 units = 30 units total width
Camera positioned to see roughly this range
Log wraps exactly at screen edge
```

**Step 2: Sinking Animation Lerp**
```cpp
float target = isSinking ? -Config::LOG_SINK_AMOUNT : 0.0f;
sinkOffset += (target - sinkOffset) * Config::LOG_SINK_SPEED * deltaTime;
```

**State Transition:**
```
If player on log:
  isSinking = true (set by collision check)
  target = -0.07 (sinks down 0.07 units)
  
If player not on log:
  isSinking = false (didn't collide this frame)
  target = 0.0 (return to normal height)
  
sinkOffset interpolates between these states
```

**Lerp Smoothing:**
```
With LOG_SINK_SPEED = 6.0:

Player jumps on log:
  Frame 1: sinkOffset += (−0.07 - 0) * 6.0 * 0.00833 = -0.0035
  Frame 2: sinkOffset ≈ -0.007
  Frame 3: sinkOffset ≈ -0.01
  ...
  Frame 20: sinkOffset ≈ -0.065 (nearly sunk)
  
Player jumps off log:
  Frame 1: sinkOffset += (0 - (-0.065)) * 6.0 * 0.00833 = +0.0033
  Frame 2: sinkOffset ≈ -0.061
  ...
  Frame 20: sinkOffset ≈ -0.005 (nearly back up)
```

**Visual Effect:**
- Log visibly depresses when player lands (smooth sink)
- Log bounces back when player leaves (smooth rise)
- Creates physical weight impression

**Step 3: Reset Sinking Flag**
```cpp
isSinking = false;
```
- Sinking flag gets set by collision detection each frame
- Reset here for next frame's collision check
- Ensures log only sinks while collision active

### Car Movement

```cpp
    position.x += speed * deltaTime;
    if (speed > 0 && position.x >  15.0f) position.x = -15.0f;
    if (speed < 0 && position.x < -15.0f) position.x =  15.0f;
}
```

**Explanation:** Cars identical to normal logs:
- Move at constant speed (5-12 units/sec, random)
- Wrap around at ±15 boundaries
- No sinking or acceleration (car-specific logic)
- If hit by car, collision triggers instant death (DEATH_SQUISH)

### Train Rendering

```cpp
void Obstacle::render(Renderer& renderer) {
    if (!isActive) return;

    if (type == OBSTACLE_LOG) {
        glm::vec3 renderPos = position;
        renderPos.y += sinkOffset;
        renderer.drawTexturedCube(renderPos, size, "log");
        return;
    }

    if (type == OBSTACLE_TRAIN) {
        const float dir = (speed >= 0.0f) ? 1.0f : -1.0f;
        const glm::vec3 p = position;

        glm::vec3 bodyBlue (0.18f, 0.45f, 0.82f);
        glm::vec3 roofWhite(0.92f, 0.92f, 0.96f);
        glm::vec3 glassCol (0.55f, 0.78f, 0.95f);
        glm::vec3 darkWin  (0.08f, 0.09f, 0.12f);
        glm::vec3 wheelCol (0.08f, 0.08f, 0.08f);
        glm::vec3 stackCol (0.28f, 0.28f, 0.30f);
        glm::vec3 headlamp (1.00f, 0.96f, 0.70f);
        glm::vec3 redStripe(0.88f, 0.14f, 0.14f);
        glm::vec3 cowcatch (0.14f, 0.14f, 0.16f);
        glm::vec3 carBody  (0.74f, 0.78f, 0.83f);
        glm::vec3 steamCol (0.95f, 0.95f, 0.95f);

        glm::vec3 loco(p.x + dir * 7.75f, p.y, p.z);
        glm::vec3 c1  (p.x + dir * 2.85f, p.y, p.z);
        glm::vec3 c2  (p.x - dir * 2.15f, p.y, p.z);
        glm::vec3 c3  (p.x - dir * 7.15f, p.y, p.z);
```

**Explanation:**
- Complex train rendering with locomotive + 3 cars
- Each car positioned relative to train center position
- Direction determines forward-facing orientation (dir = ±1)
- Multiple color definitions for parts

### Locomotive Details

```cpp
        renderer.drawCube(loco, {4.5f, 0.58f, 0.88f}, bodyBlue);
        renderer.drawCube(loco + glm::vec3(-dir*0.65f, 0.50f, 0.0f), {3.0f, 0.36f, 0.82f}, roofWhite);
        renderer.drawCube(loco + glm::vec3(-dir*0.65f, 0.76f, 0.0f), {2.8f, 0.12f, 0.80f}, bodyBlue);
        renderer.drawCube(loco + glm::vec3(dir*2.25f, -0.18f, 0.0f),  {0.48f, 0.24f, 0.86f}, cowcatch);
        renderer.drawCube(loco + glm::vec3(dir*0.3f, -0.22f, 0.0f),   {2.8f, 0.10f, 0.90f}, redStripe);
        renderer.drawCube(loco + glm::vec3(dir*1.4f, 0.68f, 0.0f),    {0.28f, 0.36f, 0.28f}, stackCol);
        renderer.drawCube(loco + glm::vec3(dir*1.4f, 0.93f, 0.0f),    {0.42f, 0.10f, 0.42f}, stackCol);
        renderer.drawCube(loco + glm::vec3(dir*1.4f, 1.08f, 0.0f),    {0.34f, 0.22f, 0.34f}, steamCol);
        renderer.drawCube(loco + glm::vec3(dir*1.55f, 1.26f, 0.0f),   {0.22f, 0.16f, 0.22f}, steamCol);
```

**Explanation:**
- Main body + roof layers (layered for 3D effect)
- Cow catcher (front bumper)
- Red stripe along side
- Smokestack + steam cloud

### Headlights

```cpp
        renderer.drawCubeEmissive(loco + glm::vec3(dir*2.27f,  0.14f,  0.20f), {0.18f, 0.18f, 0.14f}, headlamp);
        renderer.drawCubeEmissive(loco + glm::vec3(dir*2.27f,  0.14f, -0.20f), {0.18f, 0.18f, 0.14f}, headlamp);

        if (renderer.isNightMode()) {
            renderer.drawHeadlightBeam(loco + glm::vec3(dir*2.27f, 0.14f,  0.20f),
                                       dir, 1.40f, 0.50f, 8.0f);
            renderer.drawHeadlightBeam(loco + glm::vec3(dir*2.27f, 0.14f, -0.20f),
                                       dir, 1.40f, 0.50f, 8.0f);
        }
```

**Explanation:**
- Two bright headlamps (emissive = glowing unaffected by day/night)
- At night: light beams shine forward from headlamps

### Train Windows

```cpp
        renderer.drawCube(loco + glm::vec3(-dir*0.30f, 0.50f,  0.437f), {1.30f, 0.30f, 0.02f}, stackCol);
        renderer.drawCube(loco + glm::vec3(-dir*0.30f, 0.50f, -0.437f), {1.30f, 0.30f, 0.02f}, stackCol);
        renderer.drawCube(loco + glm::vec3(-dir*0.30f, 0.50f,  0.445f), {1.20f, 0.26f, 0.04f}, glassCol);
        renderer.drawCube(loco + glm::vec3(-dir*0.30f, 0.50f, -0.445f), {1.20f, 0.26f, 0.04f}, glassCol);
        renderer.drawCube(loco + glm::vec3( dir*2.24f, 0.48f,  0.0f), {0.04f, 0.24f, 0.60f}, glassCol);
        renderer.drawCube(loco + glm::vec3(-dir*2.24f, 0.46f,  0.0f), {0.04f, 0.20f, 0.50f}, glassCol);
```

**Explanation:**
- Side windows: dark frame first, then tinted glass protruding
- Front window: thin glass pane on front
- Rear cab window: smaller window in back

### Wheels

```cpp
        float locoWheelX[3] = { dir*1.6f, dir*0.2f, -dir*1.1f };
        for (int w = 0; w < 3; w++) {
            renderer.drawCube(loco + glm::vec3(locoWheelX[w], -0.32f,  0.43f), {0.28f, 0.28f, 0.22f}, wheelCol);
            // ... more wheels
        }
```

**Explanation:** 3 wheels per side (6 total per locomotive) for steam train authenticity

---

## 10. collision.cpp - Collision Detection

### AABB Collision

```cpp
bool Collision::checkAABB(glm::vec3 posA, glm::vec3 sizeA, glm::vec3 posB, glm::vec3 sizeB) {
    glm::vec3 hitA = sizeA * Config::HITBOX_PADDING;
    glm::vec3 hitB = sizeB * Config::HITBOX_PADDING;

    return (std::abs(posA.x - posB.x) < (hitA.x + hitB.x) / 2.0f) &&
           (std::abs(posA.y - posB.y) < (hitA.y + hitB.y) / 2.0f) &&
           (std::abs(posA.z - posB.z) < (hitA.z + hitB.z) / 2.0f);
}
```

**Detailed Explanation - AABB Collision Detection Algorithm:**

**Algorithm Name**: Axis-Aligned Bounding Box (AABB) Collision Detection

**Theory:**
- AABB = rectangular prism with edges parallel to coordinate axes (X, Y, Z)
- **Separating Axis Theorem (SAT)**: Two AABBs don't collide if you can find an axis where they don't overlap
- For AABBs, you only need to check 3 axes (X, Y, Z)

**Inputs:**
- `posA`, `sizeA`: Center position and half-dimensions of box A (e.g., player character)
- `posB`, `sizeB`: Center position and half-dimensions of box B (e.g., obstacle)
- `Config::HITBOX_PADDING`: 0.8f (80% of visual size)

**Step 1: Reduce Hitbox Sizes**
```cpp
glm::vec3 hitA = sizeA * Config::HITBOX_PADDING;
glm::vec3 hitB = sizeB * Config::HITBOX_PADDING;
```
- **Purpose**: Forgiveness factor for player
- **Effect**: Shrinks collision boxes to 80% of visual size
- **Example**: Player displays as 0.8×0.8×0.8 cube, collision tested with 0.64×0.64×0.64
- **Result**: Player can "graze" obstacles and survive (feels fair)
- **Math**: If visual size is [2.0, 1.0, 1.0], hit size becomes [1.6, 0.8, 0.8]

**Step 2-4: Check X, Y, Z Axes Independently**
```cpp
std::abs(posA.x - posB.x) < (hitA.x + hitB.x) / 2.0f
std::abs(posA.y - posB.y) < (hitA.y + hitB.y) / 2.0f
std::abs(posA.z - posB.z) < (hitA.z + hitB.z) / 2.0f
```

**What This Math Does:**

For X-axis (Y and Z identical):
```
Box A: center at posA.x, half-width hitA.x
       extends from posA.x - hitA.x to posA.x + hitA.x
       
Box B: center at posB.x, half-width hitB.x
       extends from posB.x - hitB.x to posB.x + hitB.x

Distance between centers: |posA.x - posB.x|
Sum of half-widths: (hitA.x + hitB.x) / 2.0f

COLLISION occurs if: distance < combined half-width
```

**Visual Example with X-axis:**

```
Case 1: COLLISION (overlapping)
Box A:    [===]           center=2, half-width=1, extends 1-3
Box B:           [===]    center=4, half-width=1, extends 3-5
Distance = |2-4| = 2
Sum of half-widths = (1+1)/2 = 1
Check: 2 < 1? NO... wait, this should be COLLISION!

Actually, the formula should be understood as:
Sum of half-widths = (hitA.x + hitB.x) / 2.0f
Wait, that's wrong. Let me recalculate:

hitA.x and hitB.x are already HALF-dimensions (distance from center to edge)
So sum of half-widths = hitA.x + hitB.x (not divided by 2)

But the code has: (hitA.x + hitB.x) / 2.0f
This is INCORRECT formula! But let's trace it:

Actually, looking at the code more carefully:
sizeA and sizeB are FULL dimensions
hitA = sizeA * 0.8 are still FULL dimensions (80% of size)

So if sizeA = [0.8, 0.8, 0.8] (visual size)
Then hitA = [0.64, 0.64, 0.64] (hit size)
Collision check: |posA.x - posB.x| < (0.64 + 0.64) / 2.0f = 0.64

This checks if centers are within 0.64 of each other (half the collision width)
Which means collision extents from -0.32 to +0.32 from each center
That's 0.64 total width, matching the size.

Actually this is correct! The (hitA.x + hitB.x) / 2.0 represents:
- Sum of collision sizes: 0.64 + 0.64 = 1.28
- Divided by 2: = 0.64
- This is the maximum distance between centers for collision
```

**More Concrete Example:**

```
Player (Box A):
  Position: [0, 0, 0]
  Size: [0.8, 0.8, 0.8]
  Hit Size: [0.64, 0.64, 0.64]
  Extends from [-0.32, -0.32, -0.32] to [+0.32, +0.32, +0.32]

Car (Box B):
  Position: [0.5, 0.3, 0.2]
  Size: [1.5, 0.6, 0.8]
  Hit Size: [1.2, 0.48, 0.64]
  Extends from [-0.1, 0.06, -0.12] to [+1.1, 0.54, +0.52]

Check X-axis:
  Distance: |0 - 0.5| = 0.5
  Combined: (0.64 + 1.2) / 2.0 = 0.92
  Collision? 0.5 < 0.92? YES ✓

Check Y-axis:
  Distance: |0 - 0.3| = 0.3
  Combined: (0.64 + 0.48) / 2.0 = 0.56
  Collision? 0.3 < 0.56? YES ✓

Check Z-axis:
  Distance: |0 - 0.2| = 0.2
  Combined: (0.64 + 0.64) / 2.0 = 0.64
  Collision? 0.2 < 0.64? YES ✓

RESULT: ALL three axes overlap → COLLISION DETECTED!
```

**Why All Three Must Be True (AND logic):**

Two boxes only collide if they overlap on ALL three axes simultaneously:
- If separated on X: no collision (boxes are side-by-side)
- If separated on Y: no collision (boxes are up/down)
- If separated on Z: no collision (boxes are front/back)
- Only if overlapping X AND Y AND Z: collision

**Return Value:**
```cpp
return (check_x) && (check_y) && (check_z)
```
- Returns `true` if collision detected
- Returns `false` if boxes don't touch

**Performance Characteristics:**
- **Time Complexity**: O(1) - just 3 distance checks and comparisons
- **Space Complexity**: O(1) - only temporary vectors
- **Fast**: ~nanosecond operation, suitable for frame-by-frame checking
- **Used For**: Every obstacle in every lane per frame (hundreds of calls/frame)

**Limitations of AABB:**
- Only works for axis-aligned boxes (no rotated collision)
- Can't handle spheres or irregular shapes
- Conservative (may report collision before actual visual contact)
- But fine for grid-based Frogger-style game where everything aligned to grid

---

## 11. coin.cpp - Collectible Items

```cpp
Coin::Coin(glm::vec3 pos) {
    position = pos;
    collected = false;
}

void Coin::render(Renderer& renderer) {
    if (collected) return;

    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.85f, 0.0f); // gold color

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);

    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glRotatef(time * 120.0f, 0, 1, 0);

    glutSolidSphere(0.2f, 20, 20);

    glPopMatrix();
}
```

**Explanation:**
- Each coin rendered as rotating gold sphere
- Rotates 120°/second around Y axis (spins)
- Hidden once collected
- Uses GLUT built-in sphere for simplicity

---

## 12. camera.cpp - View Management

### Camera Initialization

```cpp
Camera::Camera() {
    isLocked = false;
    currentPresetIndex = 0;
    overlayFadeTimer = 0.0f;

    presets.push_back({glm::radians(20.0f), glm::radians(45.0f), 11.0f});
    presets.push_back({0.0f, glm::radians(25.0f), 4.0f});
    presets.push_back({0.0f, glm::radians(85.0f), 18.0f});

    targetYaw = currentYaw = presets[0].yaw;
    targetPitch = currentPitch = presets[0].pitch;
    targetRadius = currentRadius = presets[0].radius;
}
```

**Explanation:** Three camera presets:
1. Default: yaw 20°, pitch 45°, distance 11 (angled rear view)
2. Chase: yaw 0°, pitch 25°, distance 4 (close behind)
3. Overhead: yaw 0°, pitch 85°, distance 18 (nearly top-down)

### Camera Update

```cpp
void Camera::update(float deltaTime, int windowWidth, int windowHeight, glm::vec3 targetPos) {
    if (overlayFadeTimer > 0.0f) {
        overlayFadeTimer -= deltaTime;
    }

    currentYaw += (targetYaw - currentYaw) * params.lerpSpeed * deltaTime;
    currentPitch += (targetPitch - currentPitch) * params.lerpSpeed * deltaTime;
    currentRadius += (targetRadius - currentRadius) * params.lerpSpeed * deltaTime;

    offset.x = currentRadius * cos(currentPitch) * sin(currentYaw);
    offset.y = currentRadius * sin(currentPitch);
    offset.z = currentRadius * cos(currentPitch) * cos(currentYaw);

    position = targetPos + offset;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)windowWidth / (double)windowHeight, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}
```

**Detailed Explanation - Spherical Coordinate Orbital Camera:**

**Algorithm Type**: Exponential lerp smoothing with spherical coordinate transformation

**Purpose**: Smooth orbiting camera that follows player from multiple preset angles

**Inputs:**
- `deltaTime`: Frame time in seconds (e.g., 0.00833s for 120 FPS)
- `windowWidth`, `windowHeight`: 800×600 for aspect ratio calculation
- `targetPos`: Player position to orbit around (e.g., [0, 0, -5])
- `targetYaw`, `targetPitch`, `targetRadius`: Preset angles/distance

**Step 1: Update Overlay Fade Timer**
```cpp
if (overlayFadeTimer > 0.0f) {
    overlayFadeTimer -= deltaTime;
}
```
- Tracks time remaining to show camera control hint UI
- When <= 0, hint disappears
- Reset whenever player moves mouse (see processMouseDrag)

**Step 2: Exponential Lerp - Yaw (Horizontal Angle)**
```cpp
currentYaw += (targetYaw - currentYaw) * params.lerpSpeed * deltaTime;
```

**Lerp Formula Breakdown:**

```
Linear lerp:     y = a + t(b-a)    where t ∈ [0,1]
This formula:    currentYaw += (targetYaw - currentYaw) * speed * dt

Rearranging:     currentYaw = currentYaw + (targetYaw - currentYaw) * speed * dt
                             = currentYaw(1 - speed*dt) + targetYaw(speed*dt)
                             = weighted average of current and target
```

**Example with params.lerpSpeed = 7.0:**

```
Preset 1: targetYaw = 0 radians (directly in front of player)
Current frame: currentYaw = 3.14 (behind, opposite side)
deltaTime = 0.00833s (120 FPS)

Frame 1: currentYaw += (0 - 3.14) * 7.0 * 0.00833
         currentYaw += -3.14 * 0.05831
         currentYaw += -0.1831
         currentYaw = 3.14 - 0.1831 = 2.957

Frame 2: currentYaw = 2.957 + (0 - 2.957) * 7.0 * 0.00833
         currentYaw ≈ 2.777

Exponential decay: Each frame, remaining error multiplied by (1 - 0.05831) ≈ 0.94157
After ~10 frames: error ≈ 0.94157^10 ≈ 0.57 (57% remaining)
After ~40 frames: error ≈ 0.94157^40 ≈ 0.09 (9% remaining)
Time to ~90% there: 40 * 0.00833 ≈ 0.33 seconds
```

**Why Exponential Rather Than Linear:**
- Exponential feels smooth (decelerating arrival, not abrupt)
- Always moving, but slowing down as it approaches target
- Linear would appear jerky (constant speed then sudden stop)

**Step 3: Exponential Lerp - Pitch (Vertical Angle)**
```cpp
currentPitch += (targetPitch - currentPitch) * params.lerpSpeed * deltaTime;
```
- Same formula as yaw
- Vertical component of camera orbit
- params.lerpSpeed = 7.0 (same speed as yaw for consistent motion)

**Step 4: Exponential Lerp - Radius (Distance)**
```cpp
currentRadius += (targetRadius - currentRadius) * params.lerpSpeed * deltaTime;
```
- Same formula as yaw/pitch
- Distance from camera to player
- All three use same lerp speed for synchronized smooth orbit

**Step 5: Spherical to Cartesian Conversion**

Spherical coordinates use:
- `yaw` (θ): horizontal angle, 0 to 2π
- `pitch` (φ): vertical angle, typically -π/2 to π/2
- `radius` (r): distance from origin

Cartesian conversion formulas:
```
x = r * cos(pitch) * sin(yaw)
y = r * sin(pitch)
z = r * cos(pitch) * cos(yaw)
```

```cpp
offset.x = currentRadius * cos(currentPitch) * sin(currentYaw);
offset.y = currentRadius * sin(currentPitch);
offset.z = currentRadius * cos(currentPitch) * cos(currentYaw);
```

**Complete Example with Concrete Numbers:**

```
Player position: targetPos = [0, 0, -5]
Camera parameters:
  - yaw = 45° = π/4 radians ≈ 0.785
  - pitch = 30° = π/6 radians ≈ 0.524
  - radius = 12 units

cos(0.524) ≈ 0.866 (pitch)
sin(0.524) ≈ 0.5
cos(0.785) ≈ 0.707 (yaw)
sin(0.785) ≈ 0.707

Calculation:
  offset.x = 12 * 0.866 * 0.707 ≈ 7.34
  offset.y = 12 * 0.5 = 6.0
  offset.z = 12 * 0.866 * 0.707 ≈ 7.34

Final position:
  camera.position = [0, 0, -5] + [7.34, 6.0, 7.34]
  camera.position ≈ [7.34, 6.0, 2.34]

Result: Camera positioned northeast (45°), elevated 6 units, distance 12 from player
```

**Why Spherical Coordinates:**
- Natural for orbiting: just change angles/radius
- More intuitive than trying to move in Cartesian directly
- Guarantees consistent distance from target (radius)
- Can preset meaningful positions (front-left-up, back-right-down, etc.)

**Preset Angles (Typical Game):**

```
Preset 1 (Default):
  yaw = -π/3 (-60°)    → Southwest
  pitch = π/4 (45°)    → Elevated
  radius = 15

Preset 2 (Side):
  yaw = 0 (0°)         → Front
  pitch = π/6 (30°)    → Slightly elevated
  radius = 12

Preset 3 (Top-down):
  yaw = π/4 (45°)      → Northeast
  pitch = π/2.5 (72°)  → Very elevated
  radius = 14

All cycle smoothly between presets via lerp
```

**Step 6: Apply Offset to Get World Position**
```cpp
position = targetPos + offset;
```
- `offset`: Vector from player to camera (in world space)
- `position`: Final camera location
- Result: Camera always orbits around player, maintaining distance

**Step 7: Set Up Perspective Projection**
```cpp
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluPerspective(45.0, (double)windowWidth / (double)windowHeight, 1.0, 100.0);
glMatrixMode(GL_MODELVIEW);
```

**What This Does:**

- `glMatrixMode(GL_PROJECTION)`: Switch to projection matrix (camera lens)
- `glLoadIdentity()`: Reset to identity (no transformation)
- `gluPerspective(45.0, 4/3, 1.0, 100.0)`:
  - FOV (Field of View): 45°
  - Aspect ratio: 800/600 = 4/3 (typical 4:3 screen ratio)
  - Near plane: 1.0 units (closest visible distance)
  - Far plane: 100.0 units (farthest visible distance)
  
  Creates frustum (pyramid-shaped viewing volume):
  ```
              Far Plane (100 units away)
             /                          \
            /                            \
           /                              \
          /         45° FOV                \
         /                                  \
  Camera ------- Near Plane (1 unit)
  ```

- `glMatrixMode(GL_MODELVIEW)`: Switch back to model view for rendering

**Rendering Pipeline Impact:**
- Objects outside 1-100 unit range won't be rendered (clipped)
- 45° FOV gives wide but not ultra-wide view
- 4:3 ratio matches 800×600 without stretching

### Camera Apply

```cpp
void Camera::apply() {
    glLoadIdentity();
    gluLookAt(
        position.x, position.y, position.z,
        position.x - offset.x, 0.0f, position.z - offset.z, 
        0.0f, 1.0f, 0.0f
    );
}
```

**Detailed Explanation - View Matrix Setup:**

**Purpose**: Transform world coordinates to camera-relative coordinates

**gluLookAt Parameters:**

```cpp
gluLookAt(eye_x, eye_y, eye_z,       // Camera position
          center_x, center_y, center_z,  // Look-at point
          up_x, up_y, up_z)           // Up direction
```

**Step 1: Camera Position (Eye)**
```cpp
position.x, position.y, position.z
```
- Where camera is located (calculated in update())
- Example: [7.34, 6.0, 2.34]

**Step 2: Look-At Point (Center)**
```cpp
position.x - offset.x, 0.0f, position.z - offset.z
```

**Calculation:**
```
position = [7.34, 6.0, 2.34]
offset = [7.34, 6.0, 7.34]

Look-at = [7.34 - 7.34, 0.0, 2.34 - 7.34]
        = [0.0, 0.0, -5.0]

Result: Camera looks toward player base (player position at ground level)
Y is always 0, so camera looks horizontally at player feet, not face
```

**Why This Offset:**
- Player is at `targetPos` (e.g., [0, 0, -5])
- Camera position is `position = targetPos + offset` (e.g., [7.34, 6.0, 2.34])
- Look-at must be offset back: `position - offset = targetPos`
- But Y forced to 0 (ground level of player)

**Step 3: Up Vector**
```cpp
0.0f, 1.0f, 0.0f
```
- Defines which direction is "up" in view
- (0, 1, 0) = positive Y axis
- Keeps world oriented normally (not tilted sideways)
- Alternative: (0, 0, 1) would make Z seem up (wrong for this game)

**View Matrix Construction:**
gluLookAt creates matrix that:
1. Translates world so camera is at origin
2. Rotates world so camera looks down -Z axis
3. Rotates world so up vector aligns with Y

Result: All rendering done in camera-relative coordinates

**Complete Vision Flow:**
```
1. Camera at [7.34, 6.0, 2.34]
2. Looks toward [0, 0, -5] (player base)
3. Up direction is Y axis
4. World transforms so camera sees everything from this perspective
5. Renderer draws all objects in camera view
```

### Mouse Drag Handling

```cpp
void Camera::processMouseDrag(float deltaX, float deltaY) {
    if (isLocked) return;

    targetYaw += deltaX * params.mouseSensitivity;
    targetPitch += deltaY * params.mouseSensitivity;
    targetPitch = std::max(params.minPitch, std::min(targetPitch, params.maxPitch));

    overlayFadeTimer = params.overlayShowTime;
}
```

**Detailed Explanation - Interactive Camera Control:**

**Inputs:**
- `deltaX`: Horizontal mouse movement (pixels, e.g., 50 pixels right)
- `deltaY`: Vertical mouse movement (pixels, e.g., 30 pixels down)
- `params.mouseSensitivity`: Conversion factor (typically 0.01)

**Step 1: Early Exit if Locked**
```cpp
if (isLocked) return;
```
- In gameplay, camera might be locked to preset (no manual control)
- During game-over or menu, could be locked/unlocked

**Step 2: Update Yaw (Horizontal Rotation)**
```cpp
targetYaw += deltaX * params.mouseSensitivity;
```

**Example:**
```
Mouse moved 100 pixels right
mouseSensitivity = 0.01

targetYaw += 100 * 0.01 = 1.0 radian added
Result: Camera rotates ~57° to the right

If targetYaw was π/4, now π/4 + 1.0 ≈ 1.785 radians
Current camera smoothly lerps toward this new yaw over ~0.3 seconds
```

**Unlimited Yaw:** No wrapping, yaw can exceed 2π (camera keeps rotating)

**Step 3: Update Pitch (Vertical Rotation)**
```cpp
targetPitch += deltaY * params.mouseSensitivity;
```

**Example:**
```
Mouse moved 50 pixels down
mouseSensitivity = 0.01

targetPitch += 50 * 0.01 = 0.5 radians added
Result: Camera rotates ~29° downward

Note: down on screen = positive pitch (elevation)
```

**Step 4: Clamp Pitch**
```cpp
targetPitch = std::max(params.minPitch, std::min(targetPitch, params.maxPitch));
```

**Purpose:** Prevent camera from going upside-down or looking straight up/down

**Typical Limits:**
```
minPitch = -π/3 (-60°)  → Can look down to 60° below horizontal
maxPitch = π/2 + 0.2    → Can look up to almost vertical (but not quite)

Result: Camera stays in reasonable ranges
```

**Step 5: Reset UI Fade Timer**
```cpp
overlayFadeTimer = params.overlayShowTime;
```
- When player moves camera, show UI hint overlay again
- Timer resets to initial value (e.g., 3 seconds)
- Overlay gradually fades out over time
- Next mouse movement resets timer again

---

## 13. shader.cpp - Graphics Shader Management

```cpp
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR: Shader file not successfully read!" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    unsigned int vertex, fragment;
    
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}
```

**Detailed Explanation - GPU Shader Compilation Pipeline:**

**Algorithm Type**: Two-stage compilation (source → binary) with linking

**Purpose**: Load shader source from disk and compile for GPU execution

**Shader Architecture Background:**

Modern graphics rendering uses two programmable shaders:
- **Vertex Shader**: Runs once per vertex (3D position), transforms to camera space
- **Fragment Shader**: Runs once per pixel, determines final color
- Together they define entire rendering pipeline for this game

**Step 1: File I/O - Load Vertex Shader Source**
```cpp
std::ifstream vShaderFile;
vShaderFile.open(vertexPath);  // Open assets/shaders/vertex.glsl
std::stringstream vShaderStream;
vShaderStream << vShaderFile.rdbuf();  // Read entire file into memory
vShaderFile.close();
vertexCode = vShaderStream.str();
```

**What This Does:**

```
Asset file: assets/shaders/vertex.glsl
Content (example):
  #version 120
  varying vec3 vertexColor;
  
  void main() {
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
    vertexColor = gl_Color;
  }

Result: vertexCode = entire file as C++ string
Size: ~200-500 bytes for typical game shader
```

**Why Stringstream:**
- Reads file chunks into buffer
- `rdbuf()` gets entire content at once
- More efficient than reading line-by-line

**Step 2: File I/O - Load Fragment Shader Source**
```cpp
std::ifstream fShaderFile;
fShaderFile.open(fragmentPath);  // Open assets/shaders/fragment.glsl
std::stringstream fShaderStream;
fShaderStream << fShaderFile.rdbuf();
fShaderFile.close();
fragmentCode = fShaderStream.str();
```

**Same process as vertex shader**
Fragment shader determines pixel colors (e.g., applying textures, lighting)

**Step 3: Error Handling**
```cpp
try {
    // File opening...
} catch (std::ifstream::failure& e) {
    std::cerr << "ERROR: Shader file not successfully read!" << std::endl;
}
```

**Catches errors like:**
- File not found (missing assets/shaders/vertex.glsl)
- Permission denied (can't read file)
- Disk I/O error

**If caught:** Prints error and continues (program doesn't crash, but rendering likely fails)

**Step 4: Prepare C Strings**
```cpp
const char* vShaderCode = vertexCode.c_str();
const char* fShaderCode = fragmentCode.c_str();
```

**Why:** OpenGL API expects C-style string pointers (const char*), not C++ strings
- `c_str()` returns pointer to underlying character array
- Used immediately (pointer valid during OpenGL calls)

**Step 5: Create and Compile Vertex Shader**
```cpp
unsigned int vertex;
vertex = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(vertex, 1, &vShaderCode, NULL);
glCompileShader(vertex);
```

**Step 5a: Create Shader Object**
```cpp
vertex = glCreateShader(GL_VERTEX_SHADER);
```
- `glCreateShader()` allocates GPU shader object
- Returns integer handle (e.g., 1, 2, 3...)
- `GL_VERTEX_SHADER` specifies type (vs. GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, etc.)

**Step 5b: Send Source Code to GPU**
```cpp
glShaderSource(vertex, 1, &vShaderCode, NULL);
```

**Parameters:**
- `vertex`: Shader handle from previous step
- `1`: Number of source strings (could concatenate multiple files)
- `&vShaderCode`: Pointer to pointer of source code (pass by reference)
- `NULL`: Null-terminate string (NULL means C-string is null-terminated)

**Result:** GPU driver has source code, stored in driver memory

**Step 5c: Compile Source to GPU Binary**
```cpp
glCompileShader(vertex);
```

**What Happens (Driver Does):**

```
1. Syntax check - Verify valid GLSL syntax
2. Parsing - Build abstract syntax tree
3. Intermediate representation - Convert to intermediate form
4. Optimization - Remove dead code, simplify expressions
5. Code generation - Convert to GPU machine code
6. Error reporting - Log any compilation errors

GPU Machine Code (Binary):
  - GPU-specific instructions (not x86 or ARM)
  - Optimized for parallel execution on GPU cores
  - Cannot be read as text (binary blob)
```

**Compilation Errors (Examples):**
```
ERROR: vertex.glsl:15: vec4 undeclared identifier 'normal'
ERROR: vertex.glsl:8: type mismatch: cannot assign vec3 to vec4
ERROR: fragment.glsl:3: main() declared with non-void return type
```
If compilation fails, shader is left in error state (won't link)

**Step 6: Create and Compile Fragment Shader**
```cpp
unsigned int fragment;
fragment = glCreateShader(GL_FRAGMENT_SHADER);
glShaderSource(fragment, 1, &fShaderCode, NULL);
glCompileShader(fragment);
```

**Identical process to vertex shader**
Same 5a→5b→5c flow, but for fragment shader

**Step 7: Create Program and Attach Shaders**
```cpp
ID = glCreateProgram();
glAttachShader(ID, vertex);
glAttachShader(ID, fragment);
```

**Step 7a: Create Program Object**
```cpp
ID = glCreateProgram();
```
- Program = container that holds multiple shaders
- `ID` stored as member variable for later use
- Handle used to select this program for rendering (glUseProgram(ID))

**Step 7b: Attach Shaders to Program**
```cpp
glAttachShader(ID, vertex);
glAttachShader(ID, fragment);
```

**What This Does:**
```
Program before:
  [empty]

After attaching vertex:
  [Vertex Shader Binary]
  
After attaching fragment:
  [Vertex Shader Binary]
  [Fragment Shader Binary]
  
Linking will connect outputs of one to inputs of next
```

**Step 8: Link Program**
```cpp
glLinkProgram(ID);
```

**Linking Process (Driver Does):**

```
1. Check compatibility - Vertex outputs must match fragment inputs
2. Resolve uniforms - Each uniform gets memory address (VRAM)
3. Optimize across stages - Cross-shader optimizations
4. Create executable - Combine all shaders into single GPU program
5. Error checking - Detect linker errors

Errors (Examples):
  ERROR: vertex shader output 'color' not used in fragment shader
  ERROR: fragment shader input 'normal' not provided by vertex shader
  ERROR: uniform 'transform' declared with incompatible types
```

**Result:** Linked binary program ready to execute on GPU

**Program Execution Pipeline (At Render Time):**

```
For each vertex in geometry:
  1. GPU runs vertex shader on this vertex
  2. Shader outputs: transformed position, colors, normals, etc.
  3. GPU interpolates outputs across triangle surface
  4. For each pixel covered:
     5. GPU runs fragment shader with interpolated inputs
     6. Shader outputs: final pixel color
     7. GPU writes to framebuffer
```

**Step 9: Clean Up Shader Objects**
```cpp
glDeleteShader(vertex);
glDeleteShader(fragment);
```

**Why Delete:**
- Shaders compiled and linked into program (GPU has binaries)
- Individual shader objects no longer needed
- Frees GPU memory (VRAM)
- Can delete safely - program retains compiled code

**Before Deletion:**
```
GPU Memory:
  Shader object (vertex binary): 4KB
  Shader object (fragment binary): 5KB
  Program object (linked binary): 6KB
  Total: 15KB
```

**After Deletion:**
```
GPU Memory:
  Program object (linked binary): 6KB
  Total: 5KB saved (vertex and fragment objects freed)
```

**Complete Compilation Example:**

```
Input:
  vertexPath = "assets/shaders/vertex.glsl"
  fragmentPath = "assets/shaders/fragment.glsl"

Execution:
  1. Load vertex.glsl from disk → 300 bytes
  2. Load fragment.glsl from disk → 250 bytes
  3. Create vertex shader object (GPU ID=1)
  4. Send source to GPU
  5. GPU compiles → GPU binary 2KB
  6. Create fragment shader object (GPU ID=2)
  7. Send source to GPU
  8. GPU compiles → GPU binary 2.5KB
  9. Create program object (GPU ID=3)
  10. Attach both shaders to program
  11. GPU links shaders → GPU binary 3KB
  12. Delete shader objects (free 2 + 2.5 = 4.5KB)
  13. Store ID=3 for later use

Output:
  - Member variable `ID` = 3 (program handle)
  - Ready to call glUseProgram(ID) for rendering
  - Vertex and fragment shaders compiled, optimized, linked
```

### Shader Usage

```cpp
void Shader::use() { 
    glUseProgram(ID); 
}

void Shader::setInt(const std::string &name, int value) const { 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
```

**Explanation:**
- `use()`: Activates shader program for rendering
- `setInt()`: Sets uniform variable (same value for all vertices/fragments)

---

# RENDERING & GRAPHICS

## 14. renderer.h - Graphics Rendering Class

### Core Rendering Functions

```cpp
void initialize();
void prepareFrame();
void loadTexture(const char* path, const std::string& name);
void drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color);
void drawCubeEmissive(glm::vec3 position, glm::vec3 scale, glm::vec3 color);
void drawCubeShaded(glm::vec3 position, glm::vec3 scale, glm::vec3 baseColor, float sunAngle);
void drawHeadlightBeam(glm::vec3 origin, float dirX, float spreadZ, float spreadY, float length);
void drawTexturedCube(glm::vec3 position, glm::vec3 scale, const std::string& textureName, float rotationY = 0.0f);
```

**Explanation:**
- `drawCube()`: Basic colored cube
- `drawCubeEmissive()`: Glowing cube (unaffected by lighting)
- `drawCubeShaded()`: Day/night shading based on sun angle
- `drawHeadlightBeam()`: 3D light cone from train headlights
- `drawTexturedCube()`: Cube with texture mapping (logs)

### Lighting & Shadows

```cpp
void drawAnimatedWater(glm::vec3 position, glm::vec3 scale, int frameTime = 0);
void drawLilypad(glm::vec3 position, glm::vec3 size, glm::vec3 centerColor, glm::vec3 edgeColor);
void drawSignalPost(glm::vec3 base, bool lightRed, bool lightGreen);

void setNightMode(bool night);
bool isNightMode() const;
void updateLighting(float currentTime);
void drawSunAndMoon(float sunProgress, bool isDayTime);

void drawShadow(glm::vec3 position, glm::vec3 size, float sunAngle, float sunAngleMagnitude, float shadowFadeFactor, LaneType laneType);
void drawCharacterShadow(glm::vec3 position, glm::vec3 size, float sunAngle, float sunAngleMagnitude, float shadowFadeFactor, LaneType laneType);
void drawObstacleShadow(glm::vec3 position, glm::vec3 size, float sunAngle, float sunAngleMagnitude, float shadowFadeFactor, LaneType laneType);
```

**Explanation:**
- Water and lilypad rendering
- Signal posts with red/green lights
- Day/night cycle control
- Shadow rendering for all object types

---

## 15. pregame.cpp - Main Menu & UI

### Menu Rendering Functions

```cpp
void PreGameManager::render(GameState state, const Chicken& player, int windowWidth, int windowHeight,
                            int eggClicks, int lastClickTime, int selectedCharacterIndex,
                            int highScore, int totalCoins, int score, int coinScore)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    int timeMs = glutGet(GLUT_ELAPSED_TIME);
    
    if (state == GAME_STATE_MAIN_MENU) {
        renderMainMenu(windowWidth, windowHeight, highScore, totalCoins);
    }
    else if (state == GAME_STATE_CHARACTER_SELECT) {
        renderCharacterSelect(windowWidth, windowHeight, selectedCharacterIndex, timeMs);
    }
    else if (state == GAME_STATE_START_SCREEN) {
        renderStartScreen(windowWidth, windowHeight, timeMs);
    }
    else if (state == GAME_STATE_GAME_OVER) {
        renderGameOver(windowWidth, windowHeight, score, coinScore, highScore, timeMs);
    }
    
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
```

**Explanation:**
- Switches to 2D orthographic projection for UI
- Disables depth testing and textures for crisp UI rendering
- Enables blending for transparency effects
- Routes to appropriate render function based on game state

### Keyboard Input

```cpp
void PreGameManager::onKeyPress(unsigned char key, GameState& state, int& selectedCharacterIndex,
                                Chicken& player, int numChars)
{
    if (state == GAME_STATE_START_SCREEN) {
        if (key == '1') player.setModel(MODEL_CHICKEN);
        if (key == '2') player.setModel(MODEL_FROG);
        if (key == '3') player.setModel(MODEL_DINO);
        if (key == '4') player.setModel(MODEL_CAT);
        if (key == '5') player.setModel(MODEL_DOG);
    }
    
    if (state == GAME_STATE_CHARACTER_SELECT && (key == '\r' || key == '\n')) {
        decltype(MODEL_CHICKEN) models[] = { MODEL_CHICKEN, MODEL_FROG, MODEL_DINO, MODEL_CAT, MODEL_DOG };
        player.setModel(models[selectedCharacterIndex]);
        state = GAME_STATE_MAIN_MENU;
    }
}
```

**Explanation:**
- Number keys select character model
- Enter on selection screen confirms choice and returns to menu

### Arrow Key Navigation

```cpp
void PreGameManager::onSpecialKey(int key, GameState& state, int& selectedCharacterIndex, int numChars)
{
    if (state != GAME_STATE_CHARACTER_SELECT) return;
    
    if (key == GLUT_KEY_LEFT)
        selectedCharacterIndex = (selectedCharacterIndex - 1 + numChars) % numChars;
    else if (key == GLUT_KEY_RIGHT)
        selectedCharacterIndex = (selectedCharacterIndex + 1) % numChars;
}
```

**Explanation:**
- Left/Right arrows cycle through character selection
- Modulo wraps around (circular list)

### Mouse Clicking

```cpp
void PreGameManager::onMouseClick(int button, int clickState, int x, int y, GameState& state,
                                  int& selectedCharacterIndex, Chicken& player, int windowWidth,
                                  int windowHeight, int& eggClicks, int& lastClickTime)
{
    if (clickState != 0) return;  // Only on button down
    
    int invertedY = windowHeight - y;
    float cx = windowWidth / 2.0f;
    float cy = windowHeight / 2.0f;
    
    // ... button hit detection ...
}
```

**Explanation:**
- Converts mouse coordinates (inverts Y for OpenGL)
- Checks if click is inside button areas
- Handles egg clicking and menu navigation

---

## Architecture Summary

The game uses a **component-based architecture** with clear separation:

1. **Entry Point** (main.cpp): GLUT event routing
2. **Game Logic** (game.cpp): Core game loop, collision, state management
3. **Entities** (character, lane, obstacle): Data and behavior
4. **Rendering** (renderer): All 3D/2D graphics
5. **Input** (camera, pregame): User interaction handling
6. **Persistence** (save_data): Data storage

This modular design allows easy addition of new content (characters, obstacles, effects) without touching core systems.

---

## Key Algorithms Explained

### Lane Generation
Random weighted selection → width variation → buffer zones between hazard types → safe path tracking

### Collision Detection
AABB checks with padding for forgiving gameplay → specialized handling for different obstacle types

### Camera Movement
Spherical coordinate orbiting → exponential lerp smoothing → preset cycling

### Water Death Effect
Sinking → particle spawn → gravity/drag simulation → fade out

### Day/Night Cycle
Normalized time → sun angle calculation → lighting updates → mode switching at horizon

---

**End of Document**