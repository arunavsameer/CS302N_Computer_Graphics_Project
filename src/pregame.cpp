#include "../include/pregame.h"
#include "../include/types.h"
#include "../include/save_data.h"
#include <cstdint>
#include <cmath>
#include <sstream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

PreGameManager::PreGameManager(int windowWidth, int windowHeight)
    : windowWidth(windowWidth), windowHeight(windowHeight)
{
}

void PreGameManager::update(float deltaTime, GameState& state, Chicken& player, int& eggClicks,
                           int& lastClickTime, int& selectedCharacterIndex)
{
    // Pre-game states don't need continuous updates
    // This is a placeholder for any time-based pre-game logic if needed
}

void PreGameManager::render(GameState state, const Chicken& player, int windowWidth, int windowHeight,
                            int eggClicks, int lastClickTime, int selectedCharacterIndex,
                            int highScore, int totalCoins, int score, int coinScore,
                            uint64_t purchasedCharacters)
{
    // Setup 2D rendering
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
        renderCharacterSelect(windowWidth, windowHeight, selectedCharacterIndex, timeMs, totalCoins, purchasedCharacters);
    }
    else if (state == GAME_STATE_START_SCREEN) {
        renderStartScreen(windowWidth, windowHeight, timeMs);
    }
    else if (state == GAME_STATE_GAME_OVER) {
        renderGameOver(windowWidth, windowHeight, score, coinScore, highScore, timeMs);
    }
    
    // Restore GL state
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

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

void PreGameManager::onSpecialKey(int key, GameState& state, int& selectedCharacterIndex, int numChars)
{
    if (state != GAME_STATE_CHARACTER_SELECT)
        return;
    if (key == GLUT_KEY_LEFT)
        selectedCharacterIndex = (selectedCharacterIndex - 1 + numChars) % numChars;
    else if (key == GLUT_KEY_RIGHT)
        selectedCharacterIndex = (selectedCharacterIndex + 1) % numChars;
}

void PreGameManager::onMouseClick(int button, int clickState, int x, int y, GameState& state,
                                  int& selectedCharacterIndex, Chicken& player, int windowWidth,
                                  int windowHeight, int& eggClicks, int& lastClickTime,
                                  uint64_t& totalCoins, uint64_t& purchasedCharacters,
                                  uint64_t highScore)
{
    if (clickState != 0) return;
    
    // Helper: is a character index owned?
    auto isOwned = [&](int ci) -> bool {
        if (ci == 0 || ci == 1) return true;           // Chicken, Frog always free
        if (ci == 2) return (purchasedCharacters & 1);  // Dino  – bit 0
        if (ci == 3) return (purchasedCharacters & 2);  // Cat   – bit 1
        if (ci == 4) return (purchasedCharacters & 4);  // Dog   – bit 2
        return true;
    };

    // Prices for each character index (0/1 = free)
    const int charPrices[5] = { 0, 0, 300, 400, 500 };
    
    int invertedY = windowHeight - y;
    float cx = windowWidth / 2.0f;
    float cy = windowHeight / 2.0f;
    
    if (state == GAME_STATE_MAIN_MENU) {
        // CHARACTERS button – bottom-left (cx=72, cy=52, w=118, h=42)
        if (x >= 13 && x <= 131 && invertedY >= 31 && invertedY <= 73) {
            state = GAME_STATE_CHARACTER_SELECT;
        }
        else {
            // Everything else starts the game
            state = GAME_STATE_START_SCREEN;
            eggClicks = 0;
        }
    }
    else if (state == GAME_STATE_CHARACTER_SELECT) {
        float carouselY  = cy + 30.0f;
        float arrowR     = 38.0f;
        float lax        = 52.0f;
        float rax        = windowWidth - 52.0f;
        int   numChars   = 5;
        int   centerChar = selectedCharacterIndex;
        bool  centerOwned = isOwned(centerChar);
        
        // Left arrow
        float dlx = x - lax, dly = invertedY - carouselY;
        if (dlx*dlx + dly*dly <= arrowR*arrowR) {
            selectedCharacterIndex = (selectedCharacterIndex - 1 + numChars) % numChars;
        }
        // Right arrow
        else {
            float drx = x - rax, dry = invertedY - carouselY;
            if (drx*drx + dry*dry <= arrowR*arrowR) {
                selectedCharacterIndex = (selectedCharacterIndex + 1) % numChars;
            }
            // Main action button (SELECT or BUY) – (cx, cy-148, w=200, h=56)
            else if (x > cx - 100 && x < cx + 100 && invertedY > cy - 176 && invertedY < cy - 120) {
                if (centerOwned) {
                    // SELECT: switch character and go to main menu
                    decltype(MODEL_CHICKEN) models[] = { MODEL_CHICKEN, MODEL_FROG, MODEL_DINO, MODEL_CAT, MODEL_DOG };
                    player.setModel(models[centerChar]);
                    state = GAME_STATE_MAIN_MENU;
                } else {
                    // BUY: deduct coins and unlock if affordable
                    int price = charPrices[centerChar];
                    if ((int64_t)totalCoins >= price) {
                        totalCoins -= price;
                        if (centerChar == 2) purchasedCharacters |= 1;
                        else if (centerChar == 3) purchasedCharacters |= 2;
                        else if (centerChar == 4) purchasedCharacters |= 4;
                        // Persist the purchase immediately — don't wait for death
                        SaveManager::saveData(totalCoins, highScore, purchasedCharacters);
                    }
                    // (if not enough coins, button does nothing visible – handled by BUY label graying)
                }
            }
            // BACK button (cx=55, cy=windowHeight-50, w=84, h=34)
            else if (x >= 13 && x <= 97 &&
                     invertedY >= windowHeight - 67 && invertedY <= windowHeight - 33) {
                state = GAME_STATE_MAIN_MENU;
            }
        }
    }
    else if (state == GAME_STATE_START_SCREEN) {
        eggClicks++;
        lastClickTime = glutGet(GLUT_ELAPSED_TIME);
        if (eggClicks >= Config::MAX_EGG_CLICKS) {
            state = GAME_STATE_PLAYING;
        }
    }
    else if (state == GAME_STATE_GAME_OVER) {
        // TRY AGAIN button (cx, cy-98, w=230, h=62)
        float btnW = 230.0f, btnH = 62.0f;
        float retryY = cy - 98.0f;
        if (x > cx - btnW/2 && x < cx + btnW/2 &&
            invertedY > retryY - btnH/2 && invertedY < retryY + btnH/2) {
            // Trigger reset in game.cpp (state will be reset via game logic)
            state = GAME_STATE_MAIN_MENU;  // This signals to game.cpp to reset
        }
    }
}

void PreGameManager::renderMainMenu(int windowWidth, int windowHeight, int highScore, int totalCoins)
{
    float cx = windowWidth / 2.0f;
    float cy = windowHeight / 2.0f;
    int timeMs = glutGet(GLUT_ELAPSED_TIME);
    
    auto fillRect = [](float x1, float y1, float x2, float y2,
                       float r, float g, float b, float a = 1.0f) {
        glColor4f(r, g, b, a);
        glBegin(GL_QUADS);
        glVertex2f(x1, y1); glVertex2f(x2, y1);
        glVertex2f(x2, y2); glVertex2f(x1, y2);
        glEnd();
    };
    
    auto drawText = [](float x, float y, const std::string& txt, void* font,
                       float r, float g, float b, float a = 1.0f, bool shadow = false) {
        if (shadow) {
            glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
            glRasterPos2f(x + 2.0f, y - 2.0f);
            for (char c : txt) glutBitmapCharacter(font, c);
        }
        glColor4f(r, g, b, a);
        glRasterPos2f(x, y);
        for (char c : txt) glutBitmapCharacter(font, c);
    };
    
    auto drawCentered = [&](float tcx, float y, const std::string& txt, void* font,
                            float r, float g, float b, bool shadow = false) {
        float w = 0; for (char c : txt) w += glutBitmapWidth(font, c);
        drawText(tcx - w / 2.0f, y, txt, font, r, g, b, 1.0f, shadow);
    };
    
    auto drawButton = [&](float bcx, float bcy, float bw, float bh,
                          float br, float bg, float bb,
                          float sr, float sg, float sb,
                          const std::string& label, void* font) {
        fillRect(bcx - bw/2 + 5, bcy - bh/2 - 5, bcx + bw/2 + 5, bcy + bh/2 - 5, 0, 0, 0, 0.45f);
        fillRect(bcx - bw/2 - 2, bcy - bh/2 - 2, bcx + bw/2 + 2, bcy + bh/2 + 2, sr*0.6f, sg*0.6f, sb*0.6f);
        fillRect(bcx - bw/2, bcy - bh/2 - 4, bcx + bw/2, bcy + bh/2, sr, sg, sb);
        fillRect(bcx - bw/2, bcy - bh/2, bcx + bw/2, bcy + bh/2, br, bg, bb);
        fillRect(bcx - bw/2 + 3, bcy + bh/2 - 9, bcx + bw/2 - 3, bcy + bh/2 - 3,
                 std::min(1.0f, br * 1.55f), std::min(1.0f, bg * 1.55f), std::min(1.0f, bb * 1.55f), 0.55f);
        float lw = 0; for (char c : label) lw += glutBitmapWidth(font, c);
        drawText(bcx - lw/2.0f, bcy - 8.0f, label, font, 1.0f, 1.0f, 1.0f, 1.0f, true);
    };
    
    auto drawCircle = [](float cx_, float cy_, float r, float red, float grn, float blu, float a, int segs = 16) {
        glColor4f(red, grn, blu, a);
        glBegin(GL_POLYGON);
        for (int i = 0; i < segs; i++) {
            float ang = i * 2.0f * 3.14159f / segs;
            glVertex2f(cx_ + r * std::cos(ang), cy_ + r * std::sin(ang));
        }
        glEnd();
    };
    
    // ── Coin bank (top-left) ─────────────────────────────────────────────
    float coinX = 38.0f, coinY = windowHeight - 38.0f;
    float pillW = 110.0f;
    fillRect(coinX - 18, coinY - 16, coinX + pillW, coinY + 16, 0.0f, 0.0f, 0.0f, 0.45f);
    fillRect(coinX - 16, coinY - 14, coinX + pillW - 2, coinY + 14, 0.12f, 0.10f, 0.02f, 0.80f);
    drawCircle(coinX, coinY, 12.0f, 1.0f, 0.82f, 0.0f, 1.0f, 14);
    drawCircle(coinX, coinY,  8.0f, 0.82f, 0.65f, 0.0f, 1.0f, 14);
    std::stringstream tc; tc << totalCoins;
    drawText(coinX + 18, coinY - 7, tc.str(), GLUT_BITMAP_HELVETICA_18, 1.0f, 0.9f, 0.1f, 1.0f, true);
    
    // ── Best score (top-right) ────────────────────────────────────────────
    std::stringstream hs; hs << "BEST  " << highScore;
    float hsW = 0; for (char c : hs.str()) hsW += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
    float hsX = windowWidth - hsW - 38.0f;
    fillRect(hsX - 14, coinY - 16, windowWidth - 12, coinY + 16, 0.0f, 0.0f, 0.0f, 0.45f);
    fillRect(hsX - 12, coinY - 14, windowWidth - 14, coinY + 14, 0.05f, 0.05f, 0.12f, 0.80f);
    glColor4f(1.0f, 0.80f, 0.0f, 1.0f);
    fillRect(hsX - 10, coinY - 9, hsX + 4,  coinY + 9, 1.0f, 0.80f, 0.0f, 1.0f);
    fillRect(hsX - 12, coinY + 5, hsX + 6,  coinY + 9, 1.0f, 0.80f, 0.0f, 1.0f);
    fillRect(hsX - 5,  coinY - 12, hsX + 0, coinY - 9, 1.0f, 0.80f, 0.0f, 1.0f);
    fillRect(hsX - 8,  coinY - 14, hsX + 4, coinY - 11, 1.0f, 0.80f, 0.0f, 1.0f);
    drawText(hsX + 8, coinY - 7, hs.str(), GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, 1.0f, true);
    
    // ── Title banner ─────────────────────────────────────────────────────
    float titleY = cy + 118.0f;
    float bw = 200.0f, bh = 30.0f;
    fillRect(cx - bw - 6, titleY - bh - 6, cx + bw + 6, titleY + bh - 2, 0, 0, 0, 0.45f);
    fillRect(cx - bw - 3, titleY - bh - 3, cx + bw + 3, titleY + bh + 3, 0.05f, 0.10f, 0.22f);
    fillRect(cx - bw - 3, titleY + bh - 2, cx + bw + 3, titleY + bh + 3, 1.0f, 0.82f, 0.0f);
    fillRect(cx - bw - 3, titleY - bh - 3, cx + bw + 3, titleY - bh + 2, 1.0f, 0.82f, 0.0f);
    glColor4f(0.12f, 0.28f, 0.58f, 0.96f);
    glBegin(GL_QUADS);
    glVertex2f(cx - bw, titleY - bh); glVertex2f(cx + bw, titleY - bh);
    glVertex2f(cx + bw, titleY);       glVertex2f(cx - bw, titleY);
    glEnd();
    glColor4f(0.20f, 0.48f, 0.88f, 0.96f);
    glBegin(GL_QUADS);
    glVertex2f(cx - bw, titleY); glVertex2f(cx + bw, titleY);
    glVertex2f(cx + bw, titleY + bh); glVertex2f(cx - bw, titleY + bh);
    glEnd();
    {
        std::string title = "CRAZY  HOPPER";
        float tw = 0; for (char c : title) tw += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, c);
        glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
        glRasterPos2f(cx - tw/2 + 3, titleY - 8); for (char c : title) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        glColor4f(0.0f, 0.2f, 0.5f, 0.8f);
        glRasterPos2f(cx - tw/2 + 1, titleY - 6); for (char c : title) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glRasterPos2f(cx - tw/2, titleY - 7); for (char c : title) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }
    
    // ── PLAY button ───────────────────────────────────────────────────────
    float playPulse = 3.0f * std::abs(std::sin(timeMs * 0.0022f));
    drawButton(cx, cy - 18.0f, 230.0f + playPulse, 64.0f,
               0.08f, 0.74f, 0.08f,
               0.04f, 0.38f, 0.04f,
               "PLAY!", GLUT_BITMAP_TIMES_ROMAN_24);
    
    // Blinking "tap anywhere" hint
    if ((timeMs / 600) % 2 == 0) {
        drawCentered(cx, cy - 100.0f, "- TAP ANYWHERE TO START -",
                     GLUT_BITMAP_HELVETICA_12, 0.88f, 0.88f, 0.88f, false);
    }
    
    // ── Characters button ────────────────────────────────────────────────
    drawButton(72.0f, 52.0f, 118.0f, 42.0f,
               0.14f, 0.44f, 0.74f,
               0.06f, 0.20f, 0.40f,
               "CHARACTERS", GLUT_BITMAP_HELVETICA_12);
}

void PreGameManager::renderCharacterSelect(int windowWidth, int windowHeight, int selectedCharacterIndex,
                                          int timeMs, int totalCoins, uint64_t purchasedCharacters)
{
    float cx = windowWidth / 2.0f;
    float cy = windowHeight / 2.0f;
    
    auto fillRect = [](float x1, float y1, float x2, float y2,
                       float r, float g, float b, float a = 1.0f) {
        glColor4f(r, g, b, a);
        glBegin(GL_QUADS);
        glVertex2f(x1, y1); glVertex2f(x2, y1);
        glVertex2f(x2, y2); glVertex2f(x1, y2);
        glEnd();
    };
    
    auto drawCentered = [](float tcx, float y, const std::string& txt, void* font,
                           float r, float g, float b, bool shadow = false) {
        if (shadow) {
            glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
            glRasterPos2f(tcx, y - 2.0f);
            float w = 0; for (char c : txt) w += glutBitmapWidth(font, c);
            glRasterPos2f(tcx - w / 2.0f + 2.0f, y - 2.0f);
            for (char c : txt) glutBitmapCharacter(font, c);
        }
        glColor4f(r, g, b, 1.0f);
        float w = 0; for (char c : txt) w += glutBitmapWidth(font, c);
        glRasterPos2f(tcx - w / 2.0f, y);
        for (char c : txt) glutBitmapCharacter(font, c);
    };
    
    auto drawButton = [&](float bcx, float bcy, float bw, float bh,
                          float br, float bg, float bb,
                          float sr, float sg, float sb,
                          const std::string& label, void* font) {
        fillRect(bcx - bw/2 + 5, bcy - bh/2 - 5, bcx + bw/2 + 5, bcy + bh/2 - 5, 0, 0, 0, 0.45f);
        fillRect(bcx - bw/2 - 2, bcy - bh/2 - 2, bcx + bw/2 + 2, bcy + bh/2 + 2, sr*0.6f, sg*0.6f, sb*0.6f);
        fillRect(bcx - bw/2, bcy - bh/2 - 4, bcx + bw/2, bcy + bh/2, sr, sg, sb);
        fillRect(bcx - bw/2, bcy - bh/2, bcx + bw/2, bcy + bh/2, br, bg, bb);
        fillRect(bcx - bw/2 + 3, bcy + bh/2 - 9, bcx + bw/2 - 3, bcy + bh/2 - 3,
                 std::min(1.0f, br * 1.55f), std::min(1.0f, bg * 1.55f), std::min(1.0f, bb * 1.55f), 0.55f);
        float lw = 0; for (char c : label) lw += glutBitmapWidth(font, c);
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        glRasterPos2f(bcx - lw/2.0f + 2, bcy - 8.0f - 2); for (char c : label) glutBitmapCharacter(font, c);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glRasterPos2f(bcx - lw/2.0f, bcy - 8.0f); for (char c : label) glutBitmapCharacter(font, c);
    };
    
    auto drawCircle = [](float cx_, float cy_, float r, float red, float grn, float blu, float a, int segs = 16) {
        glColor4f(red, grn, blu, a);
        glBegin(GL_POLYGON);
        for (int i = 0; i < segs; i++) {
            float ang = i * 2.0f * 3.14159f / segs;
            glVertex2f(cx_ + r * std::cos(ang), cy_ + r * std::sin(ang));
        }
        glEnd();
    };
    
    // Helper: is a character owned?
    auto isOwned = [&](int ci) -> bool {
        if (ci == 0 || ci == 1) return true;
        if (ci == 2) return (purchasedCharacters & 1) != 0; // Dino
        if (ci == 3) return (purchasedCharacters & 2) != 0; // Cat
        if (ci == 4) return (purchasedCharacters & 4) != 0; // Dog
        return true;
    };
    const int charPrices[5] = { 0, 0, 300, 400, 500 };
    bool centerOwned = isOwned(selectedCharacterIndex);

    // Dark vignette overlay
    fillRect(0, 0, windowWidth, windowHeight, 0.02f, 0.05f, 0.12f, 0.82f);
    
    // ── Title ────────────────────────────────────────────────────────────
    {
        std::string hdr = "CHOOSE  YOUR  HOPPER";
        float tw = 0; for (char c : hdr) tw += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, c);
        fillRect(cx - tw/2 - 20, windowHeight - 72, cx + tw/2 + 20, windowHeight - 32, 0.05f, 0.12f, 0.28f, 0.85f);
        fillRect(cx - tw/2 - 20, windowHeight - 74, cx + tw/2 + 20, windowHeight - 70, 1.0f, 0.82f, 0.0f);
        fillRect(cx - tw/2 - 20, windowHeight - 34, cx + tw/2 + 20, windowHeight - 30, 1.0f, 0.82f, 0.0f);
        drawCentered(cx, windowHeight - 57, hdr, GLUT_BITMAP_TIMES_ROMAN_24, 1.0f, 1.0f, 1.0f, true);
    }

    // ── Coin balance (top-right of screen) ───────────────────────────────
    {
        std::stringstream tc; tc << totalCoins;
        float pillRight = windowWidth - 16.0f;
        float pillCy    = windowHeight - 52.0f;
        float cw = 0; for (char c : tc.str()) cw += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        float pillLeft = pillRight - cw - 48.0f;
        fillRect(pillLeft - 2, pillCy - 16, pillRight + 2, pillCy + 16, 0.0f, 0.0f, 0.0f, 0.45f);
        fillRect(pillLeft,     pillCy - 14, pillRight,     pillCy + 14, 0.12f, 0.10f, 0.02f, 0.85f);
        // Coin icon
        float icx2 = pillLeft + 16.0f;
        drawCircle(icx2, pillCy, 10.0f, 1.0f, 0.82f, 0.0f, 1.0f, 14);
        drawCircle(icx2, pillCy,  6.5f, 0.82f, 0.65f, 0.0f, 1.0f, 14);
        drawCentered(pillLeft + 28.0f + cw/2.0f, pillCy - 7.0f, tc.str(),
                     GLUT_BITMAP_HELVETICA_18, 1.0f, 0.9f, 0.1f, true);
    }
    
    // ── Character name (with lock indicator) ─────────────────────────────
    const char* charNames[] = { "CHICKEN", "FROG", "DINO", "CAT", "DOG" };
    float nameAlpha = 0.65f + 0.35f * std::abs(std::sin(timeMs * 0.0038f));
    {
        std::string nm = charNames[selectedCharacterIndex];
        if (!centerOwned) nm = std::string("\xF0\x9F\x94\x92 ") + nm; // show "locked" in name
        // Use the lock word instead of emoji for GLUT compatibility
        if (!centerOwned) nm = charNames[selectedCharacterIndex] + std::string("  [LOCKED]");
        float nw = 0; for (char c : nm) nw += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, c);
        float nameR = centerOwned ? 1.0f : 0.9f;
        float nameG = centerOwned ? 0.85f : 0.55f;
        float nameB = centerOwned ? 0.0f  : 0.15f;
        glColor4f(nameR, nameG, nameB, nameAlpha);
        glRasterPos2f(cx - nw/2, windowHeight - 108.0f);
        for (char c : nm) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }
    
    // ── Carousel cards ───────────────────────────────────────────────────
    float carouselY = cy + 30.0f;
    float offsets[5] = { -310.0f, -170.0f,   0.0f, 170.0f, 310.0f };
    float sizes[5]   = {   48.0f,   68.0f, 105.0f,  68.0f,  48.0f };
    int   numChars   = 5;
    
    for (int slot = 0; slot < 5; slot++) {
        int ci = ((selectedCharacterIndex - 2 + slot) % numChars + numChars) % numChars;
        bool isCenter = (slot == 2);
        bool owned = isOwned(ci);
        drawCharacterIcon(cx + offsets[slot], carouselY, sizes[slot], ci, isCenter, timeMs, owned);
    }
    
    // ── Navigation arrows ─────────────────────────────────────────────────
    float arrowR = 38.0f;
    float lax = 52.0f, rax = windowWidth - 52.0f, ay = carouselY;
    
    // Left arrow
    drawCircle(lax, ay, arrowR + 3, 0.0f, 0.0f, 0.0f, 0.5f);
    drawCircle(lax, ay, arrowR,     0.12f, 0.32f, 0.62f, 0.90f);
    drawCircle(lax, ay, arrowR - 4, 0.18f, 0.44f, 0.80f, 0.70f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(lax + 14.0f, ay - 18.0f);
    glVertex2f(lax - 16.0f, ay);
    glVertex2f(lax + 14.0f, ay + 18.0f);
    glEnd();
    
    // Right arrow
    drawCircle(rax, ay, arrowR + 3, 0.0f, 0.0f, 0.0f, 0.5f);
    drawCircle(rax, ay, arrowR,     0.12f, 0.32f, 0.62f, 0.90f);
    drawCircle(rax, ay, arrowR - 4, 0.18f, 0.44f, 0.80f, 0.70f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(rax - 14.0f, ay - 18.0f);
    glVertex2f(rax + 16.0f, ay);
    glVertex2f(rax - 14.0f, ay + 18.0f);
    glEnd();
    
    // Dot indicators
    float dotY = carouselY - 100.0f;
    for (int d = 0; d < numChars; d++) {
        float dotX = cx + (d - 2) * 22.0f;
        bool active = (d == selectedCharacterIndex);
        bool dOwned = isOwned(d);
        float r = active ? (dOwned ? 1.0f : 0.85f) : 0.45f;
        float g = active ? (dOwned ? 0.84f : 0.30f) : 0.45f;
        float b = active ? (dOwned ? 0.0f  : 0.10f) : 0.45f;
        drawCircle(dotX, dotY, active ? 7.0f : 4.5f, r, g, b, 1.0f, 12);
        // Small lock dot for unowned characters
        if (!dOwned && !active) {
            drawCircle(dotX, dotY, 3.0f, 0.7f, 0.4f, 0.0f, 0.9f, 8);
        }
    }

    // ── Price badge (shown only for locked center character) ─────────────
    if (!centerOwned) {
        int price = charPrices[selectedCharacterIndex];
        std::stringstream ps; ps << price << " COINS";
        float badgeAlpha = 0.75f + 0.25f * std::abs(std::sin(timeMs * 0.003f));
        float badgeCy = cy - 110.0f;
        float bw2 = 0; for (char c : ps.str()) bw2 += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        float badgeHalfW = bw2 / 2.0f + 22.0f;
        fillRect(cx - badgeHalfW + 4, badgeCy - 18, cx + badgeHalfW + 4, badgeCy + 14, 0, 0, 0, 0.45f);
        fillRect(cx - badgeHalfW,     badgeCy - 16, cx + badgeHalfW,     badgeCy + 16, 0.25f, 0.18f, 0.02f, 0.85f);
        fillRect(cx - badgeHalfW + 2, badgeCy + 8,  cx + badgeHalfW - 2, badgeCy + 14, 0.6f, 0.45f, 0.0f, 0.4f);
        // Mini coin icon next to price
        drawCircle(cx - badgeHalfW + 12, badgeCy - 1, 7.0f, 1.0f, 0.82f, 0.0f, 1.0f, 12);
        drawCircle(cx - badgeHalfW + 12, badgeCy - 1, 4.5f, 0.82f, 0.65f, 0.0f, 1.0f, 12);
        bool canAfford = (totalCoins >= price);
        float prR = canAfford ? 1.0f : 0.9f;
        float prG = canAfford ? 0.88f : 0.40f;
        float prB = canAfford ? 0.1f  : 0.10f;
        glColor4f(prR, prG, prB, badgeAlpha);
        glRasterPos2f(cx - bw2/2, badgeCy - 8);
        for (char c : ps.str()) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    
    // ── SELECT / BUY button ──────────────────────────────────────────────
    if (centerOwned) {
        // Green SELECT button
        drawButton(cx, cy - 148.0f, 200.0f, 56.0f,
                   0.08f, 0.74f, 0.08f,
                   0.04f, 0.38f, 0.04f,
                   "SELECT!", GLUT_BITMAP_TIMES_ROMAN_24);
    } else {
        // Orange BUY button — dims if not enough coins
        bool canAfford = (totalCoins >= charPrices[selectedCharacterIndex]);
        float br = canAfford ? 0.90f : 0.45f;
        float bg = canAfford ? 0.52f : 0.35f;
        float bb = canAfford ? 0.02f : 0.08f;
        float sr = canAfford ? 0.50f : 0.25f;
        float sg = canAfford ? 0.28f : 0.18f;
        float sb = canAfford ? 0.01f : 0.04f;
        std::stringstream buyLabel;
        buyLabel << "BUY  " << charPrices[selectedCharacterIndex];
        drawButton(cx, cy - 148.0f, 200.0f, 56.0f, br, bg, bb, sr, sg, sb,
                   buyLabel.str(), GLUT_BITMAP_TIMES_ROMAN_24);
        // "Not enough coins" hint
        if (!canAfford) {
            drawCentered(cx, cy - 196.0f, "Not enough coins!",
                         GLUT_BITMAP_HELVETICA_12, 1.0f, 0.3f, 0.3f, false);
        }
    }
    
    // ── Back button ──────────────────────────────────────────────────────
    drawButton(55.0f, windowHeight - 50.0f, 84.0f, 34.0f,
               0.35f, 0.35f, 0.45f,
               0.18f, 0.18f, 0.24f,
               "< BACK", GLUT_BITMAP_HELVETICA_12);
}

void PreGameManager::renderStartScreen(int windowWidth, int windowHeight, int timeMs)
{
    float cx = windowWidth / 2.0f;
    float cy = windowHeight / 2.0f;
    
    if ((timeMs / 300) % 2 == 0) {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        std::string txt = "TAP EGG TO HATCH!";
        float w = 0; for (char c : txt) w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        glRasterPos2f(cx - w/2, cy + 100.0f);
        for (char c : txt) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void PreGameManager::renderGameOver(int windowWidth, int windowHeight, int score, int coinScore,
                                   int highScore, int timeMs)
{
    float cx = windowWidth / 2.0f;
    float cy = windowHeight / 2.0f;
    
    auto fillRect = [](float x1, float y1, float x2, float y2,
                       float r, float g, float b, float a = 1.0f) {
        glColor4f(r, g, b, a);
        glBegin(GL_QUADS);
        glVertex2f(x1, y1); glVertex2f(x2, y1);
        glVertex2f(x2, y2); glVertex2f(x1, y2);
        glEnd();
    };
    
    auto drawCentered = [](float tcx, float y, const std::string& txt, void* font,
                           float r, float g, float b, bool shadow = false) {
        if (shadow) {
            glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
            glRasterPos2f(tcx, y - 2.0f);
            float w = 0; for (char c : txt) w += glutBitmapWidth(font, c);
            glRasterPos2f(tcx - w / 2.0f + 2.0f, y - 2.0f);
            for (char c : txt) glutBitmapCharacter(font, c);
        }
        glColor4f(r, g, b, 1.0f);
        float w = 0; for (char c : txt) w += glutBitmapWidth(font, c);
        glRasterPos2f(tcx - w / 2.0f, y);
        for (char c : txt) glutBitmapCharacter(font, c);
    };
    
    auto drawButton = [&](float bcx, float bcy, float bw, float bh,
                          float br, float bg, float bb,
                          float sr, float sg, float sb,
                          const std::string& label, void* font) {
        fillRect(bcx - bw/2 + 5, bcy - bh/2 - 5, bcx + bw/2 + 5, bcy + bh/2 - 5, 0, 0, 0, 0.45f);
        fillRect(bcx - bw/2 - 2, bcy - bh/2 - 2, bcx + bw/2 + 2, bcy + bh/2 + 2, sr*0.6f, sg*0.6f, sb*0.6f);
        fillRect(bcx - bw/2, bcy - bh/2 - 4, bcx + bw/2, bcy + bh/2, sr, sg, sb);
        fillRect(bcx - bw/2, bcy - bh/2, bcx + bw/2, bcy + bh/2, br, bg, bb);
        fillRect(bcx - bw/2 + 3, bcy + bh/2 - 9, bcx + bw/2 - 3, bcy + bh/2 - 3,
                 std::min(1.0f, br * 1.55f), std::min(1.0f, bg * 1.55f), std::min(1.0f, bb * 1.55f), 0.55f);
        float lw = 0; for (char c : label) lw += glutBitmapWidth(font, c);
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        glRasterPos2f(bcx - lw/2.0f + 2, bcy - 8.0f - 2); for (char c : label) glutBitmapCharacter(font, c);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glRasterPos2f(bcx - lw/2.0f, bcy - 8.0f); for (char c : label) glutBitmapCharacter(font, c);
    };
    
    // Full dark vignette
    fillRect(0, 0, windowWidth, windowHeight, 0.0f, 0.0f, 0.0f, 0.52f);
    
    // Panel
    float pW = 340.0f, pH = 230.0f, pY = cy + 22.0f;
    fillRect(cx - pW/2 + 7, pY - pH/2 - 7, cx + pW/2 + 7, pY + pH/2 - 7, 0, 0, 0, 0.75f);
    fillRect(cx - pW/2 - 4, pY - pH/2 - 4, cx + pW/2 + 4, pY + pH/2 + 4, 0.65f, 0.08f, 0.08f, 0.92f);
    fillRect(cx - pW/2, pY - pH/2, cx + pW/2, pY + pH/2, 0.04f, 0.04f, 0.11f, 0.97f);
    fillRect(cx - pW/2 + 4, pY + pH/2 - 12, cx + pW/2 - 4, pY + pH/2 - 4, 0.45f, 0.08f, 0.08f, 0.55f);
    
    // GAME OVER heading
    drawCentered(cx, pY + 88.0f, "GAME  OVER", GLUT_BITMAP_TIMES_ROMAN_24, 1.0f, 0.22f, 0.22f, true);
    
    // Divider
    fillRect(cx - 130, pY + 62, cx + 130, pY + 65, 0.55f, 0.10f, 0.10f, 0.80f);
    
    // Score row
    std::stringstream sText; sText << "SCORE      " << score;
    drawCentered(cx, pY + 36.0f, sText.str(), GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f, true);
    
    // Coins row
    std::stringstream cText; cText << "+" << coinScore << "  COINS";
    drawCentered(cx, pY + 6.0f, cText.str(), GLUT_BITMAP_HELVETICA_18, 1.0f, 0.85f, 0.0f, true);
    
    // New best banner
    if (score > 0 && score == highScore) {
        float pulse = 0.7f + 0.3f * std::abs(std::sin(timeMs * 0.005f));
        fillRect(cx - 110, pY - 22, cx + 110, pY - 4, 0.6f * pulse, 0.5f * pulse, 0.0f, 0.60f);
        drawCentered(cx, pY - 18.0f, "* NEW BEST! *", GLUT_BITMAP_HELVETICA_12,
                     1.0f * pulse, 0.9f * pulse, 0.0f, false);
    }
    
    // TRY AGAIN button
    drawButton(cx, cy - 98.0f, 230.0f, 62.0f,
               0.08f, 0.72f, 0.08f,
               0.04f, 0.36f, 0.04f,
               "TRY  AGAIN", GLUT_BITMAP_TIMES_ROMAN_24);
}

void PreGameManager::drawCharacterIcon(float icx, float icy, float sz, int ci, bool selected, int timeMs, bool owned)
{
    auto fillRect = [](float x1, float y1, float x2, float y2,
                       float r, float g, float b, float a = 1.0f) {
        glColor4f(r, g, b, a);
        glBegin(GL_QUADS);
        glVertex2f(x1, y1); glVertex2f(x2, y1);
        glVertex2f(x2, y2); glVertex2f(x1, y2);
        glEnd();
    };
    
    float s = sz / 100.0f;
    float cw = sz * 1.25f, ch = sz * 1.65f;
    
    // Card drop shadow
    fillRect(icx - cw/2 + 6, icy - ch/2 - 6, icx + cw/2 + 6, icy + ch/2 - 6, 0, 0, 0, 0.45f);
    
    // Gold animated border for selected card
    if (selected) {
        float pulse = 0.65f + 0.35f * std::abs(std::sin(timeMs * 0.0035f));
        fillRect(icx - cw/2 - 5, icy - ch/2 - 5, icx + cw/2 + 5, icy + ch/2 + 5,
                 1.0f * pulse, 0.82f * pulse, 0.0f);
    }
    
    // Card fill
    if (selected)
        fillRect(icx - cw/2, icy - ch/2, icx + cw/2, icy + ch/2, 0.12f, 0.30f, 0.58f, 0.92f);
    else
        fillRect(icx - cw/2, icy - ch/2, icx + cw/2, icy + ch/2, 0.06f, 0.12f, 0.25f, 0.80f);
    
    // Inner top sheen
    fillRect(icx - cw/2 + 3, icy + ch/2 - 10, icx + cw/2 - 3, icy + ch/2 - 3,
        1.0f, 1.0f, 1.0f, selected ? 0.12f : 0.06f);
        
    switch (ci) {
        
    // ── CHICKEN (Front-facing Voxel Style) ──────────────────────────
        case 0: {
            float wht[3] = {1.0f, 1.0f, 1.0f};      // White body
            float owt[3] = {0.92f, 0.92f, 0.88f};   // Off-white shading
            float org[3] = {1.0f, 0.50f, 0.05f};    // Orange beak/feet
            float ord[3] = {0.85f, 0.40f, 0.02f};   // Dark orange
            float red[3] = {0.85f, 0.10f, 0.10f};   // Red comb
            float pnk[3] = {1.0f, 0.20f, 0.55f};    // Pink wattle
            float yel[3] = {0.95f, 0.85f, 0.10f};   // Yellow eyes
            float blk[3] = {0.05f, 0.05f, 0.05f};   // Black pupils

            // Legs & Feet (Orange/Dark Orange)
            glColor4f(org[0], org[1], org[2], 1.0f);
            glBegin(GL_QUADS);
            // Left Leg
            glVertex2f(icx - 14*s, icy - 45*s); glVertex2f(icx - 6*s, icy - 45*s);
            glVertex2f(icx - 6*s, icy - 20*s); glVertex2f(icx - 14*s, icy - 20*s);
            // Right Leg
            glVertex2f(icx + 6*s, icy - 45*s); glVertex2f(icx + 14*s, icy - 45*s);
            glVertex2f(icx + 14*s, icy - 20*s); glVertex2f(icx + 6*s, icy - 20*s);
            glEnd();

            // Toes
            glColor4f(ord[0], ord[1], ord[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 18*s, icy - 50*s); glVertex2f(icx - 2*s, icy - 50*s);
            glVertex2f(icx - 2*s, icy - 45*s); glVertex2f(icx - 18*s, icy - 45*s);
            glVertex2f(icx + 2*s, icy - 50*s); glVertex2f(icx + 18*s, icy - 50*s);
            glVertex2f(icx + 18*s, icy - 45*s); glVertex2f(icx + 2*s, icy - 45*s);
            glEnd();

            // Body Plump (White/Off-white belly)
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 26*s, icy - 15*s); glVertex2f(icx + 26*s, icy - 15*s);
            glVertex2f(icx + 26*s, icy + 20*s); glVertex2f(icx - 26*s, icy + 20*s);
            glEnd();
            glColor4f(owt[0], owt[1], owt[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 20*s, icy - 22*s); glVertex2f(icx + 20*s, icy - 22*s);
            glVertex2f(icx + 20*s, icy - 5*s);  glVertex2f(icx - 20*s, icy - 5*s);
            glEnd();

            // Wings (Off-white edges)
            glColor4f(owt[0], owt[1], owt[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 32*s, icy - 5*s); glVertex2f(icx - 24*s, icy - 5*s);
            glVertex2f(icx - 24*s, icy + 12*s); glVertex2f(icx - 32*s, icy + 12*s);
            glVertex2f(icx + 24*s, icy - 5*s); glVertex2f(icx + 32*s, icy - 5*s);
            glVertex2f(icx + 32*s, icy + 12*s); glVertex2f(icx + 24*s, icy + 12*s);
            glEnd();

            // Head (White)
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 18*s, icy + 15*s); glVertex2f(icx + 18*s, icy + 15*s);
            glVertex2f(icx + 18*s, icy + 42*s); glVertex2f(icx - 18*s, icy + 42*s);
            glEnd();

            // Cheeks (Off-white)
            glColor4f(owt[0], owt[1], owt[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 24*s, icy + 18*s); glVertex2f(icx - 16*s, icy + 18*s);
            glVertex2f(icx - 16*s, icy + 30*s); glVertex2f(icx - 24*s, icy + 30*s);
            glVertex2f(icx + 16*s, icy + 18*s); glVertex2f(icx + 24*s, icy + 18*s);
            glVertex2f(icx + 24*s, icy + 30*s); glVertex2f(icx + 16*s, icy + 30*s);
            glEnd();

            // Red Comb
            glColor4f(red[0], red[1], red[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 6*s, icy + 42*s); glVertex2f(icx + 6*s, icy + 42*s);
            glVertex2f(icx + 6*s, icy + 54*s); glVertex2f(icx - 6*s, icy + 54*s);
            glVertex2f(icx - 14*s, icy + 42*s); glVertex2f(icx - 6*s, icy + 42*s);
            glVertex2f(icx - 6*s, icy + 50*s);  glVertex2f(icx - 14*s, icy + 50*s);
            glVertex2f(icx + 6*s, icy + 42*s);  glVertex2f(icx + 14*s, icy + 42*s);
            glVertex2f(icx + 14*s, icy + 50*s); glVertex2f(icx + 6*s, icy + 50*s);
            glEnd();

            // Wattle (Pink)
            glColor4f(pnk[0], pnk[1], pnk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 8*s, icy + 10*s); glVertex2f(icx + 8*s, icy + 10*s);
            glVertex2f(icx + 8*s, icy + 22*s); glVertex2f(icx - 8*s, icy + 22*s);
            glEnd();

            // Beak (Orange)
            glColor4f(org[0], org[1], org[2], 1.0f);
            glBegin(GL_TRIANGLES);
            glVertex2f(icx - 10*s, icy + 28*s); glVertex2f(icx + 10*s, icy + 28*s);
            glVertex2f(icx, icy + 20*s);
            glEnd();

            // Eyes (Yellow base)
            glColor4f(yel[0], yel[1], yel[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 22*s, icy + 30*s); glVertex2f(icx - 12*s, icy + 30*s);
            glVertex2f(icx - 12*s, icy + 40*s); glVertex2f(icx - 22*s, icy + 40*s);
            glVertex2f(icx + 12*s, icy + 30*s); glVertex2f(icx + 22*s, icy + 30*s);
            glVertex2f(icx + 22*s, icy + 40*s); glVertex2f(icx + 12*s, icy + 40*s);
            glEnd();

            // Pupils & Shine
            glColor4f(blk[0], blk[1], blk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 18*s, icy + 32*s); glVertex2f(icx - 14*s, icy + 32*s);
            glVertex2f(icx - 14*s, icy + 38*s); glVertex2f(icx - 18*s, icy + 38*s);
            glVertex2f(icx + 14*s, icy + 32*s); glVertex2f(icx + 18*s, icy + 32*s);
            glVertex2f(icx + 18*s, icy + 38*s); glVertex2f(icx + 14*s, icy + 38*s);
            glEnd();
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 17*s, icy + 35*s); glVertex2f(icx - 15*s, icy + 35*s);
            glVertex2f(icx - 15*s, icy + 37*s); glVertex2f(icx - 17*s, icy + 37*s);
            glVertex2f(icx + 15*s, icy + 35*s); glVertex2f(icx + 17*s, icy + 35*s);
            glVertex2f(icx + 17*s, icy + 37*s); glVertex2f(icx + 15*s, icy + 37*s);
            glEnd();
            break;
        }
        // ── FROG (Front-facing Voxel Style) ─────────────────────────────
        case 1: {
            float grn[3] = {0.35f, 0.70f, 0.30f}; // Base green
            float grd[3] = {0.20f, 0.50f, 0.15f}; // Dark green
            float grm[3] = {0.28f, 0.60f, 0.22f}; // Mid green
            float bly[3] = {0.85f, 0.90f, 0.70f}; // Pale belly
            float wht[3] = {1.0f, 1.0f, 1.0f};    // White
            float blk[3] = {0.05f, 0.05f, 0.05f}; // Black
            float gld[3] = {0.90f, 0.75f, 0.10f}; // Gold eyes
            float mth[3] = {0.18f, 0.44f, 0.12f}; // Dark mouth
            float red[3] = {0.90f, 0.10f, 0.10f}; // Tongue

            // Frog sits very low (squashed posture)
            float yb = -30.0f * s; // Shift the whole model down

            // Back Legs (Wide Stance)
            glColor4f(grd[0], grd[1], grd[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 48*s, icy + yb + 5*s); glVertex2f(icx - 30*s, icy + yb + 5*s);
            glVertex2f(icx - 30*s, icy + yb + 25*s); glVertex2f(icx - 48*s, icy + yb + 25*s);
            glVertex2f(icx + 30*s, icy + yb + 5*s); glVertex2f(icx + 48*s, icy + yb + 5*s);
            glVertex2f(icx + 48*s, icy + yb + 25*s); glVertex2f(icx + 30*s, icy + yb + 25*s);
            glEnd();

            // Front Arms
            glColor4f(grm[0], grm[1], grm[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 42*s, icy + yb - 5*s); glVertex2f(icx - 26*s, icy + yb - 5*s);
            glVertex2f(icx - 26*s, icy + yb + 15*s); glVertex2f(icx - 42*s, icy + yb + 15*s);
            glVertex2f(icx + 26*s, icy + yb - 5*s); glVertex2f(icx + 42*s, icy + yb - 5*s);
            glVertex2f(icx + 42*s, icy + yb + 15*s); glVertex2f(icx + 26*s, icy + yb + 15*s);
            glEnd();

            // Webbed Feet / Hands
            glColor4f(bly[0], bly[1], bly[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 46*s, icy + yb - 10*s); glVertex2f(icx - 22*s, icy + yb - 10*s);
            glVertex2f(icx - 22*s, icy + yb - 5*s);  glVertex2f(icx - 46*s, icy + yb - 5*s);
            glVertex2f(icx + 22*s, icy + yb - 10*s); glVertex2f(icx + 46*s, icy + yb - 10*s);
            glVertex2f(icx + 46*s, icy + yb - 5*s);  glVertex2f(icx + 22*s, icy + yb - 5*s);
            glEnd();

            // Main Body (Green)
            glColor4f(grn[0], grn[1], grn[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 36*s, icy + yb + 5*s); glVertex2f(icx + 36*s, icy + yb + 5*s);
            glVertex2f(icx + 36*s, icy + yb + 28*s); glVertex2f(icx - 36*s, icy + yb + 28*s);
            glEnd();

            // Belly (Pale)
            glColor4f(bly[0], bly[1], bly[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 30*s, icy + yb - 2*s); glVertex2f(icx + 30*s, icy + yb - 2*s);
            glVertex2f(icx + 30*s, icy + yb + 16*s); glVertex2f(icx - 30*s, icy + yb + 16*s);
            glEnd();

            // Head (Wide)
            glColor4f(grn[0], grn[1], grn[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 32*s, icy + yb + 28*s); glVertex2f(icx + 32*s, icy + yb + 28*s);
            glVertex2f(icx + 32*s, icy + yb + 46*s); glVertex2f(icx - 32*s, icy + yb + 46*s);
            glEnd();

            // Mouth Line (Dark Green)
            glColor4f(mth[0], mth[1], mth[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 26*s, icy + yb + 30*s); glVertex2f(icx + 26*s, icy + yb + 30*s);
            glVertex2f(icx + 26*s, icy + yb + 32*s); glVertex2f(icx - 26*s, icy + yb + 32*s);
            glEnd();

            // Tongue (Red protruding)
            glColor4f(red[0], red[1], red[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 6*s, icy + yb + 20*s); glVertex2f(icx + 6*s, icy + yb + 20*s);
            glVertex2f(icx + 6*s, icy + yb + 30*s); glVertex2f(icx - 6*s, icy + yb + 30*s);
            glEnd();

            // Bulging Eyes (Dome)
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 34*s, icy + yb + 42*s); glVertex2f(icx - 14*s, icy + yb + 42*s);
            glVertex2f(icx - 14*s, icy + yb + 60*s); glVertex2f(icx - 34*s, icy + yb + 60*s);
            glVertex2f(icx + 14*s, icy + yb + 42*s); glVertex2f(icx + 34*s, icy + yb + 42*s);
            glVertex2f(icx + 34*s, icy + yb + 60*s); glVertex2f(icx + 14*s, icy + yb + 60*s);
            glEnd();

            // Iris (Gold) & Pupils (Black) & Shine
            glColor4f(gld[0], gld[1], gld[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 30*s, icy + yb + 46*s); glVertex2f(icx - 18*s, icy + yb + 46*s);
            glVertex2f(icx - 18*s, icy + yb + 56*s); glVertex2f(icx - 30*s, icy + yb + 56*s);
            glVertex2f(icx + 18*s, icy + yb + 46*s); glVertex2f(icx + 30*s, icy + yb + 46*s);
            glVertex2f(icx + 30*s, icy + yb + 56*s); glVertex2f(icx + 18*s, icy + yb + 56*s);
            glEnd();
            glColor4f(blk[0], blk[1], blk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 26*s, icy + yb + 48*s); glVertex2f(icx - 22*s, icy + yb + 48*s);
            glVertex2f(icx - 22*s, icy + yb + 54*s); glVertex2f(icx - 26*s, icy + yb + 54*s);
            glVertex2f(icx + 22*s, icy + yb + 48*s); glVertex2f(icx + 26*s, icy + yb + 48*s);
            glVertex2f(icx + 26*s, icy + yb + 54*s); glVertex2f(icx + 22*s, icy + yb + 54*s);
            glEnd();
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 23*s, icy + yb + 52*s); glVertex2f(icx - 21*s, icy + yb + 52*s);
            glVertex2f(icx - 21*s, icy + yb + 54*s); glVertex2f(icx - 23*s, icy + yb + 54*s);
            glVertex2f(icx + 25*s, icy + yb + 52*s); glVertex2f(icx + 27*s, icy + yb + 52*s);
            glVertex2f(icx + 27*s, icy + yb + 54*s); glVertex2f(icx + 25*s, icy + yb + 54*s);
            glEnd();
            break;
        }

        // ── DINO (Front-facing Voxel Style) ─────────────────────────────
        case 2: {
            // Colors matching character_dino.cpp
            float brn[3] = {0.65f, 0.42f, 0.18f}; // Main brown
            float brd[3] = {0.48f, 0.30f, 0.12f}; // Dark brown
            float tan[3] = {0.88f, 0.76f, 0.54f}; // Tan snout/belly
            float pnk[3] = {1.0f, 0.60f, 0.70f};  // Pink tongue
            float amb[3] = {0.90f, 0.60f, 0.10f}; // Amber eyes

            // Tail peeking out from the right side
            glColor4f(brd[0], brd[1], brd[2], 1.0f);
            glBegin(GL_TRIANGLES);
            glVertex2f(icx + 18*s, icy - 10*s);
            glVertex2f(icx + 40*s, icy - 26*s);
            glVertex2f(icx + 18*s, icy - 30*s);
            glEnd();

            // Legs (Shin)
            glColor4f(brd[0], brd[1], brd[2], 1.0f); 
            glBegin(GL_QUADS); 
            // Left
            glVertex2f(icx - 22*s, icy - 48*s); glVertex2f(icx - 8*s, icy - 48*s);
            glVertex2f(icx - 8*s, icy - 30*s); glVertex2f(icx - 22*s, icy - 30*s);
            // Right
            glVertex2f(icx + 8*s, icy - 48*s); glVertex2f(icx + 22*s, icy - 48*s);
            glVertex2f(icx + 22*s, icy - 30*s); glVertex2f(icx + 8*s, icy - 30*s);
            glEnd();

            // Legs (Thigh)
            glColor4f(brn[0], brn[1], brn[2], 1.0f); 
            glBegin(GL_QUADS);
            // Left
            glVertex2f(icx - 24*s, icy - 36*s); glVertex2f(icx - 6*s, icy - 36*s);
            glVertex2f(icx - 6*s, icy - 20*s); glVertex2f(icx - 24*s, icy - 20*s);
            // Right
            glVertex2f(icx + 6*s, icy - 36*s); glVertex2f(icx + 24*s, icy - 36*s);
            glVertex2f(icx + 24*s, icy - 20*s); glVertex2f(icx + 6*s, icy - 20*s);
            glEnd();

            // Toe Claws (White)
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glBegin(GL_QUADS); 
            // Left claws
            glVertex2f(icx - 22*s, icy - 52*s); glVertex2f(icx - 18*s, icy - 52*s);
            glVertex2f(icx - 18*s, icy - 48*s); glVertex2f(icx - 22*s, icy - 48*s);
            glVertex2f(icx - 17*s, icy - 52*s); glVertex2f(icx - 13*s, icy - 52*s);
            glVertex2f(icx - 13*s, icy - 48*s); glVertex2f(icx - 17*s, icy - 48*s);
            glVertex2f(icx - 12*s, icy - 52*s); glVertex2f(icx - 8*s, icy - 52*s);
            glVertex2f(icx - 8*s, icy - 48*s); glVertex2f(icx - 12*s, icy - 48*s);
            // Right claws
            glVertex2f(icx + 8*s, icy - 52*s); glVertex2f(icx + 12*s, icy - 52*s);
            glVertex2f(icx + 12*s, icy - 48*s); glVertex2f(icx + 8*s, icy - 48*s);
            glVertex2f(icx + 13*s, icy - 52*s); glVertex2f(icx + 17*s, icy - 52*s);
            glVertex2f(icx + 17*s, icy - 48*s); glVertex2f(icx + 13*s, icy - 48*s);
            glVertex2f(icx + 18*s, icy - 52*s); glVertex2f(icx + 22*s, icy - 52*s);
            glVertex2f(icx + 22*s, icy - 48*s); glVertex2f(icx + 18*s, icy - 48*s);
            glEnd();

            // Main Body
            glColor4f(brn[0], brn[1], brn[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 20*s, icy - 26*s); glVertex2f(icx + 20*s, icy - 26*s);
            glVertex2f(icx + 20*s, icy + 12*s); glVertex2f(icx - 20*s, icy + 12*s);
            glEnd();

            // Belly (Tan)
            glColor4f(tan[0], tan[1], tan[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 14*s, icy - 26*s); glVertex2f(icx + 14*s, icy - 26*s);
            glVertex2f(icx + 14*s, icy + 8*s); glVertex2f(icx - 14*s, icy + 8*s);
            glEnd();

            // Little Arms
            glColor4f(brn[0], brn[1], brn[2], 1.0f);
            glBegin(GL_QUADS);
            // Left
            glVertex2f(icx - 30*s, icy - 2*s); glVertex2f(icx - 20*s, icy - 2*s);
            glVertex2f(icx - 20*s, icy + 8*s); glVertex2f(icx - 30*s, icy + 8*s);
            // Right
            glVertex2f(icx + 20*s, icy - 2*s); glVertex2f(icx + 30*s, icy - 2*s);
            glVertex2f(icx + 30*s, icy + 8*s); glVertex2f(icx + 20*s, icy + 8*s);
            glEnd();

            // Hand Claws
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glBegin(GL_QUADS);
            // Left
            glVertex2f(icx - 32*s, icy + 0*s); glVertex2f(icx - 30*s, icy + 0*s);
            glVertex2f(icx - 30*s, icy + 6*s); glVertex2f(icx - 32*s, icy + 6*s);
            // Right
            glVertex2f(icx + 30*s, icy + 0*s); glVertex2f(icx + 32*s, icy + 0*s);
            glVertex2f(icx + 32*s, icy + 6*s); glVertex2f(icx + 30*s, icy + 6*s);
            glEnd();

            // Head (Brown Top)
            glColor4f(brn[0], brn[1], brn[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 24*s, icy + 10*s); glVertex2f(icx + 24*s, icy + 10*s);
            glVertex2f(icx + 24*s, icy + 48*s); glVertex2f(icx - 24*s, icy + 48*s);
            glEnd();

            // Snout / Lower Jaw (Tan)
            glColor4f(tan[0], tan[1], tan[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 26*s, icy + 6*s); glVertex2f(icx + 26*s, icy + 6*s);
            glVertex2f(icx + 26*s, icy + 24*s); glVertex2f(icx - 26*s, icy + 24*s);
            glEnd();

            // Teeth
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glBegin(GL_QUADS);
            // Outer left/right
            glVertex2f(icx - 20*s, icy + 2*s); glVertex2f(icx - 16*s, icy + 2*s);
            glVertex2f(icx - 16*s, icy + 6*s); glVertex2f(icx - 20*s, icy + 6*s);
            glVertex2f(icx + 16*s, icy + 2*s); glVertex2f(icx + 20*s, icy + 2*s);
            glVertex2f(icx + 20*s, icy + 6*s); glVertex2f(icx + 16*s, icy + 6*s);
            // Inner left/right
            glVertex2f(icx - 12*s, icy + 2*s); glVertex2f(icx - 8*s, icy + 2*s);
            glVertex2f(icx - 8*s, icy + 6*s); glVertex2f(icx - 12*s, icy + 6*s);
            glVertex2f(icx + 8*s, icy + 2*s); glVertex2f(icx + 12*s, icy + 2*s);
            glVertex2f(icx + 12*s, icy + 6*s); glVertex2f(icx + 8*s, icy + 6*s);
            glEnd();

            // Tongue (Pink)
            glColor4f(pnk[0], pnk[1], pnk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 8*s, icy - 2*s); glVertex2f(icx + 8*s, icy - 2*s);
            glVertex2f(icx + 8*s, icy + 8*s); glVertex2f(icx - 8*s, icy + 8*s);
            glEnd();

            // Eyes (Amber background)
            glColor4f(amb[0], amb[1], amb[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 24*s, icy + 30*s); glVertex2f(icx - 12*s, icy + 30*s);
            glVertex2f(icx - 12*s, icy + 42*s); glVertex2f(icx - 24*s, icy + 42*s);
            glVertex2f(icx + 12*s, icy + 30*s); glVertex2f(icx + 24*s, icy + 30*s);
            glVertex2f(icx + 24*s, icy + 42*s); glVertex2f(icx + 12*s, icy + 42*s);
            glEnd();

            // Black Slit Pupils
            glColor4f(0.05f, 0.05f, 0.05f, 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 20*s, icy + 30*s); glVertex2f(icx - 16*s, icy + 30*s);
            glVertex2f(icx - 16*s, icy + 42*s); glVertex2f(icx - 20*s, icy + 42*s);
            glVertex2f(icx + 16*s, icy + 30*s); glVertex2f(icx + 20*s, icy + 30*s);
            glVertex2f(icx + 20*s, icy + 42*s); glVertex2f(icx + 16*s, icy + 42*s);
            glEnd();

            // Eye Shine (White)
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 18*s, icy + 36*s); glVertex2f(icx - 14*s, icy + 36*s);
            glVertex2f(icx - 14*s, icy + 40*s); glVertex2f(icx - 18*s, icy + 40*s);
            glVertex2f(icx + 14*s, icy + 36*s); glVertex2f(icx + 18*s, icy + 36*s);
            glVertex2f(icx + 18*s, icy + 40*s); glVertex2f(icx + 14*s, icy + 40*s);
            glEnd();

            break;
        }
        // ── CAT (Front-facing Voxel Style) ──────────────────────────────
        case 3: {
            float wht[3] = {1.0f, 1.0f, 1.0f};     // White
            float org[3] = {0.9f, 0.5f, 0.1f};     // Orange patch
            float blkp[3]= {0.15f, 0.15f, 0.15f};  // Black/Grey patch
            float crm[3] = {0.98f, 0.96f, 0.88f};  // Cream base
            float pnk[3] = {1.0f, 0.6f, 0.7f};     // Pink details
            float grn[3] = {0.2f, 0.7f, 0.25f};    // Green eyes
            float blk[3] = {0.05f, 0.05f, 0.05f};  // Slit pupils

            // Legs (Calico mismatch: Left=White, Right=Orange)
            glBegin(GL_QUADS);
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glVertex2f(icx - 16*s, icy - 42*s); glVertex2f(icx - 6*s, icy - 42*s);
            glVertex2f(icx - 6*s, icy - 18*s);  glVertex2f(icx - 16*s, icy - 18*s);
            glColor4f(org[0], org[1], org[2], 1.0f);
            glVertex2f(icx + 6*s, icy - 42*s);  glVertex2f(icx + 16*s, icy - 42*s);
            glVertex2f(icx + 16*s, icy - 18*s); glVertex2f(icx + 6*s, icy - 18*s);
            glEnd();

            // Paws (Cream/White)
            glColor4f(crm[0], crm[1], crm[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 18*s, icy - 46*s); glVertex2f(icx - 4*s, icy - 46*s);
            glVertex2f(icx - 4*s, icy - 40*s);  glVertex2f(icx - 18*s, icy - 40*s);
            glVertex2f(icx + 4*s, icy - 46*s);  glVertex2f(icx + 18*s, icy - 46*s);
            glVertex2f(icx + 18*s, icy - 40*s); glVertex2f(icx + 4*s, icy - 40*s);
            glEnd();

            // Body Base (Cream)
            glColor4f(crm[0], crm[1], crm[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 22*s, icy - 20*s); glVertex2f(icx + 22*s, icy - 20*s);
            glVertex2f(icx + 22*s, icy + 20*s); glVertex2f(icx - 22*s, icy + 20*s);
            glEnd();

            // Body Patches
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 12*s, icy - 12*s); glVertex2f(icx + 12*s, icy - 12*s);
            glVertex2f(icx + 12*s, icy + 22*s); glVertex2f(icx - 12*s, icy + 22*s);
            glEnd();
            glColor4f(blkp[0], blkp[1], blkp[2], 1.0f); // Left shoulder dark patch
            glBegin(GL_QUADS);
            glVertex2f(icx - 24*s, icy + 4*s); glVertex2f(icx - 10*s, icy + 4*s);
            glVertex2f(icx - 10*s, icy + 18*s); glVertex2f(icx - 24*s, icy + 18*s);
            glEnd();

            // Head (White base)
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 22*s, icy + 20*s); glVertex2f(icx + 22*s, icy + 20*s);
            glVertex2f(icx + 22*s, icy + 48*s); glVertex2f(icx - 22*s, icy + 48*s);
            glEnd();

            // Forehead patch (Orange top right)
            glColor4f(org[0], org[1], org[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx, icy + 36*s); glVertex2f(icx + 22*s, icy + 36*s);
            glVertex2f(icx + 22*s, icy + 48*s); glVertex2f(icx, icy + 48*s);
            glEnd();

            // Pointy Ears (Left=Black, Right=Orange)
            glBegin(GL_TRIANGLES);
            glColor4f(blkp[0], blkp[1], blkp[2], 1.0f);
            glVertex2f(icx - 22*s, icy + 46*s); glVertex2f(icx - 8*s, icy + 46*s); glVertex2f(icx - 18*s, icy + 62*s);
            glColor4f(org[0], org[1], org[2], 1.0f);
            glVertex2f(icx + 8*s, icy + 46*s); glVertex2f(icx + 22*s, icy + 46*s); glVertex2f(icx + 18*s, icy + 62*s);
            glEnd();

            // Inner Ear (Pink)
            glColor4f(pnk[0], pnk[1], pnk[2], 1.0f);
            glBegin(GL_TRIANGLES);
            glVertex2f(icx - 18*s, icy + 48*s); glVertex2f(icx - 12*s, icy + 48*s); glVertex2f(icx - 16*s, icy + 56*s);
            glVertex2f(icx + 12*s, icy + 48*s); glVertex2f(icx + 18*s, icy + 48*s); glVertex2f(icx + 16*s, icy + 56*s);
            glEnd();

            // Muzzle & Cheeks (White)
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 12*s, icy + 22*s); glVertex2f(icx + 12*s, icy + 22*s);
            glVertex2f(icx + 12*s, icy + 30*s); glVertex2f(icx - 12*s, icy + 30*s);
            glEnd();

            // Nose (Pink)
            glColor4f(pnk[0], pnk[1], pnk[2], 1.0f);
            glBegin(GL_TRIANGLES);
            glVertex2f(icx - 4*s, icy + 28*s); glVertex2f(icx + 4*s, icy + 28*s); glVertex2f(icx, icy + 24*s);
            glEnd();

            // Tongue (Pink hanging down)
            glColor4f(pnk[0], pnk[1], pnk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 3*s, icy + 18*s); glVertex2f(icx + 3*s, icy + 18*s);
            glVertex2f(icx + 3*s, icy + 24*s); glVertex2f(icx - 3*s, icy + 24*s);
            glEnd();

            // Eyes (Green iris, slit pupil, shine)
            glColor4f(grn[0], grn[1], grn[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 18*s, icy + 32*s); glVertex2f(icx - 8*s, icy + 32*s);
            glVertex2f(icx - 8*s, icy + 40*s); glVertex2f(icx - 18*s, icy + 40*s);
            glVertex2f(icx + 8*s, icy + 32*s); glVertex2f(icx + 18*s, icy + 32*s);
            glVertex2f(icx + 18*s, icy + 40*s); glVertex2f(icx + 8*s, icy + 40*s);
            glEnd();
            glColor4f(blk[0], blk[1], blk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 14*s, icy + 33*s); glVertex2f(icx - 12*s, icy + 33*s);
            glVertex2f(icx - 12*s, icy + 39*s); glVertex2f(icx - 14*s, icy + 39*s);
            glVertex2f(icx + 12*s, icy + 33*s); glVertex2f(icx + 14*s, icy + 33*s);
            glVertex2f(icx + 14*s, icy + 39*s); glVertex2f(icx + 12*s, icy + 39*s);
            glEnd();
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 13*s, icy + 36*s); glVertex2f(icx - 10*s, icy + 36*s);
            glVertex2f(icx - 10*s, icy + 38*s); glVertex2f(icx - 13*s, icy + 38*s);
            glVertex2f(icx + 13*s, icy + 36*s); glVertex2f(icx + 16*s, icy + 36*s);
            glVertex2f(icx + 16*s, icy + 38*s); glVertex2f(icx + 13*s, icy + 38*s);
            glEnd();
            break;
        }
        // ── DOG (Front-facing Voxel Style) ──────────────────────────────
        case 4: {
            float tan[3] = {0.80f, 0.60f, 0.35f};  // Tan body
            float tnd[3] = {0.60f, 0.42f, 0.22f};  // Dark tan (ears/back)
            float crm[3] = {0.98f, 0.95f, 0.85f};  // Cream belly/muzzle
            float wht[3] = {1.0f, 1.0f, 1.0f};     // White shine
            float blk[3] = {0.10f, 0.10f, 0.10f};  // Black nose/pupils
            float pnk[3] = {1.0f, 0.6f, 0.7f};     // Pink tongue
            float brn[3] = {0.45f, 0.28f, 0.10f};  // Brown eyes

            // Legs (Tan with cream paws)
            glColor4f(tan[0], tan[1], tan[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 18*s, icy - 40*s); glVertex2f(icx - 6*s, icy - 40*s);
            glVertex2f(icx - 6*s, icy - 18*s);  glVertex2f(icx - 18*s, icy - 18*s);
            glVertex2f(icx + 6*s, icy - 40*s);  glVertex2f(icx + 18*s, icy - 40*s);
            glVertex2f(icx + 18*s, icy - 18*s); glVertex2f(icx + 6*s, icy - 18*s);
            glEnd();
            glColor4f(crm[0], crm[1], crm[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 20*s, icy - 46*s); glVertex2f(icx - 4*s, icy - 46*s);
            glVertex2f(icx - 4*s, icy - 36*s);  glVertex2f(icx - 20*s, icy - 36*s);
            glVertex2f(icx + 4*s, icy - 46*s);  glVertex2f(icx + 20*s, icy - 46*s);
            glVertex2f(icx + 20*s, icy - 36*s); glVertex2f(icx + 4*s, icy - 36*s);
            glEnd();

            // Body (Tan Barrel Chest)
            glColor4f(tan[0], tan[1], tan[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 24*s, icy - 18*s); glVertex2f(icx + 24*s, icy - 18*s);
            glVertex2f(icx + 24*s, icy + 22*s); glVertex2f(icx - 24*s, icy + 22*s);
            glEnd();

            // Belly (Cream)
            glColor4f(crm[0], crm[1], crm[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 14*s, icy - 18*s); glVertex2f(icx + 14*s, icy - 18*s);
            glVertex2f(icx + 14*s, icy + 12*s); glVertex2f(icx - 14*s, icy + 12*s);
            glEnd();

            // Head Base (Tan)
            glColor4f(tan[0], tan[1], tan[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 24*s, icy + 22*s); glVertex2f(icx + 24*s, icy + 22*s);
            glVertex2f(icx + 24*s, icy + 54*s); glVertex2f(icx - 24*s, icy + 54*s);
            glEnd();

            // Top Stripe/Brows (Dark Tan)
            glColor4f(tnd[0], tnd[1], tnd[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 10*s, icy + 48*s); glVertex2f(icx + 10*s, icy + 48*s);
            glVertex2f(icx + 10*s, icy + 54*s); glVertex2f(icx - 10*s, icy + 54*s);
            glVertex2f(icx - 20*s, icy + 46*s); glVertex2f(icx - 12*s, icy + 46*s);
            glVertex2f(icx - 12*s, icy + 50*s); glVertex2f(icx - 20*s, icy + 50*s);
            glVertex2f(icx + 12*s, icy + 46*s); glVertex2f(icx + 20*s, icy + 46*s);
            glVertex2f(icx + 20*s, icy + 50*s); glVertex2f(icx + 12*s, icy + 50*s);
            glEnd();

            // Floppy Ears (Dark Tan)
            glColor4f(tnd[0], tnd[1], tnd[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 38*s, icy + 20*s); glVertex2f(icx - 22*s, icy + 20*s);
            glVertex2f(icx - 22*s, icy + 50*s); glVertex2f(icx - 38*s, icy + 50*s);
            glVertex2f(icx + 22*s, icy + 20*s); glVertex2f(icx + 38*s, icy + 20*s);
            glVertex2f(icx + 38*s, icy + 50*s); glVertex2f(icx + 22*s, icy + 50*s);
            glEnd();

            // Inner Ear (Pink sliver)
            glColor4f(pnk[0], pnk[1], pnk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 32*s, icy + 22*s); glVertex2f(icx - 28*s, icy + 22*s);
            glVertex2f(icx - 28*s, icy + 40*s); glVertex2f(icx - 32*s, icy + 40*s);
            glVertex2f(icx + 28*s, icy + 22*s); glVertex2f(icx + 32*s, icy + 22*s);
            glVertex2f(icx + 32*s, icy + 40*s); glVertex2f(icx + 28*s, icy + 40*s);
            glEnd();

            // Muzzle (Cream)
            glColor4f(crm[0], crm[1], crm[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 16*s, icy + 20*s); glVertex2f(icx + 16*s, icy + 20*s);
            glVertex2f(icx + 16*s, icy + 34*s); glVertex2f(icx - 16*s, icy + 34*s);
            glEnd();

            // Nose (Black)
            glColor4f(blk[0], blk[1], blk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 8*s, icy + 30*s); glVertex2f(icx + 8*s, icy + 30*s);
            glVertex2f(icx + 8*s, icy + 36*s); glVertex2f(icx - 8*s, icy + 36*s);
            glEnd();

            // Tongue (Pink hanging)
            glColor4f(pnk[0], pnk[1], pnk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 6*s, icy + 8*s);  glVertex2f(icx + 6*s, icy + 8*s);
            glVertex2f(icx + 6*s, icy + 20*s); glVertex2f(icx - 6*s, icy + 20*s);
            glEnd();
            glColor4f(0.85f, 0.40f, 0.50f, 1.0f); // Tongue crease
            glBegin(GL_QUADS);
            glVertex2f(icx - 1*s, icy + 10*s); glVertex2f(icx + 1*s, icy + 10*s);
            glVertex2f(icx + 1*s, icy + 18*s); glVertex2f(icx - 1*s, icy + 18*s);
            glEnd();

            // Eyes (Brown Iris, Black Pupil, White Shine)
            glColor4f(brn[0], brn[1], brn[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 18*s, icy + 36*s); glVertex2f(icx - 8*s, icy + 36*s);
            glVertex2f(icx - 8*s, icy + 44*s);  glVertex2f(icx - 18*s, icy + 44*s);
            glVertex2f(icx + 8*s, icy + 36*s);  glVertex2f(icx + 18*s, icy + 36*s);
            glVertex2f(icx + 18*s, icy + 44*s); glVertex2f(icx + 8*s, icy + 44*s);
            glEnd();
            glColor4f(blk[0], blk[1], blk[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 14*s, icy + 38*s); glVertex2f(icx - 10*s, icy + 38*s);
            glVertex2f(icx - 10*s, icy + 42*s); glVertex2f(icx - 14*s, icy + 42*s);
            glVertex2f(icx + 10*s, icy + 38*s); glVertex2f(icx + 14*s, icy + 38*s);
            glVertex2f(icx + 14*s, icy + 42*s); glVertex2f(icx + 10*s, icy + 42*s);
            glEnd();
            glColor4f(wht[0], wht[1], wht[2], 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(icx - 13*s, icy + 40*s); glVertex2f(icx - 11*s, icy + 40*s);
            glVertex2f(icx - 11*s, icy + 42*s); glVertex2f(icx - 13*s, icy + 42*s);
            glVertex2f(icx + 13*s, icy + 40*s); glVertex2f(icx + 15*s, icy + 40*s);
            glVertex2f(icx + 15*s, icy + 42*s); glVertex2f(icx + 13*s, icy + 42*s);
            glEnd();
            break;
        }
    }

    // ── Lock overlay (drawn on top for any unowned character) ─────────────
    if (!owned) {
        // Dark semi-transparent tint over the whole card
        float cw = sz * 1.25f, ch = sz * 1.65f;
        glColor4f(0.0f, 0.0f, 0.0f, 0.62f);
        glBegin(GL_QUADS);
        glVertex2f(icx - cw/2, icy - ch/2);
        glVertex2f(icx + cw/2, icy - ch/2);
        glVertex2f(icx + cw/2, icy + ch/2);
        glVertex2f(icx - cw/2, icy + ch/2);
        glEnd();

        // ── Lock icon (centered on card) ──────────────────────────────────
        // Scale lock with card size
        float ls = sz / 100.0f;

        // Shackle (rounded U-bar drawn as 3 quads: left post, right post, top bar)
        float shW = 7.0f * ls;   // half-width of shackle interior gap
        float shT = 4.0f * ls;   // bar thickness
        float shH = 20.0f * ls;  // shackle height above lock body top

        float bodyT  = icy + 4.0f * ls;  // lock body top
        float shackleBottom = bodyT;
        float shackleTop    = bodyT + shH;

        glColor4f(0.85f, 0.80f, 0.70f, 1.0f);  // light gold/silver shackle
        glBegin(GL_QUADS);
        // Left post
        glVertex2f(icx - shW - shT, shackleBottom);
        glVertex2f(icx - shW,       shackleBottom);
        glVertex2f(icx - shW,       shackleTop);
        glVertex2f(icx - shW - shT, shackleTop);
        // Right post
        glVertex2f(icx + shW,       shackleBottom);
        glVertex2f(icx + shW + shT, shackleBottom);
        glVertex2f(icx + shW + shT, shackleTop);
        glVertex2f(icx + shW,       shackleTop);
        // Top bar
        glVertex2f(icx - shW - shT, shackleTop - shT);
        glVertex2f(icx + shW + shT, shackleTop - shT);
        glVertex2f(icx + shW + shT, shackleTop);
        glVertex2f(icx - shW - shT, shackleTop);
        glEnd();

        // Lock body (gold rectangle below shackle)
        float bodyHalfW = 18.0f * ls;
        float bodyH     = 22.0f * ls;
        float bodyBot   = bodyT - bodyH;

        // Body shadow
        glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
        glBegin(GL_QUADS);
        glVertex2f(icx - bodyHalfW + 3*ls, bodyBot - 3*ls);
        glVertex2f(icx + bodyHalfW + 3*ls, bodyBot - 3*ls);
        glVertex2f(icx + bodyHalfW + 3*ls, bodyT   - 3*ls);
        glVertex2f(icx - bodyHalfW + 3*ls, bodyT   - 3*ls);
        glEnd();
        // Body fill (amber gold)
        glColor4f(0.85f, 0.62f, 0.08f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(icx - bodyHalfW, bodyBot);
        glVertex2f(icx + bodyHalfW, bodyBot);
        glVertex2f(icx + bodyHalfW, bodyT);
        glVertex2f(icx - bodyHalfW, bodyT);
        glEnd();
        // Top sheen
        glColor4f(1.0f, 0.88f, 0.40f, 0.55f);
        glBegin(GL_QUADS);
        glVertex2f(icx - bodyHalfW + 2*ls, bodyT - 5*ls);
        glVertex2f(icx + bodyHalfW - 2*ls, bodyT - 5*ls);
        glVertex2f(icx + bodyHalfW - 2*ls, bodyT - 2*ls);
        glVertex2f(icx - bodyHalfW + 2*ls, bodyT - 2*ls);
        glEnd();

        // Keyhole (dark circle + vertical slot)
        float kcy = (bodyBot + bodyT) / 2.0f + 1.0f * ls;
        float kr  = 5.5f * ls;
        glColor4f(0.08f, 0.06f, 0.02f, 1.0f);
        glBegin(GL_POLYGON);
        for (int ki = 0; ki < 16; ki++) {
            float ang = ki * 2.0f * 3.14159f / 16;
            glVertex2f(icx + kr * std::cos(ang), kcy + kr * std::sin(ang));
        }
        glEnd();
        // Slot below keyhole
        glBegin(GL_QUADS);
        glVertex2f(icx - 2.5f*ls, bodyBot + 2*ls);
        glVertex2f(icx + 2.5f*ls, bodyBot + 2*ls);
        glVertex2f(icx + 2.5f*ls, kcy);
        glVertex2f(icx - 2.5f*ls, kcy);
        glEnd();
    }
}