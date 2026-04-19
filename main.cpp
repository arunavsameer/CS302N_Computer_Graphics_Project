#include <GL/glew.h>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#include "game.h"
#include "types.h"

#include <iostream>
#include <chrono>

Game* game = nullptr;

// State variables for tracking the mouse drag
bool isDragging = false;
int lastMouseX = -1;
int lastMouseY = -1;

// Frame rate limiting
auto lastFrameTime = std::chrono::high_resolution_clock::now();

void display() {
    game->render();
    glutSwapBuffers();
}

void idle() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - lastFrameTime;
    
    // Only update and render if enough time has passed for the next frame
    if (elapsed.count() >= Config::FRAME_TIME) {
        game->update(elapsed.count() * Config::GAME_SPEED_MULTIPLIER);
        lastFrameTime = currentTime;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // ESC
        delete game;
        exit(0);
    }
    game->onKeyPress(key);
}

// Handle initial mouse clicks
void mouseButton(int button, int state, int x, int y) {
    // 1. Pass the click to the game logic to handle the Egg and Replay buttons
    if (game != nullptr) {
        game->onMouseClick(button, state, x, y);
    }

    // 2. Existing drag logic for the camera
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

// Handle mouse movement while a button is pressed
void mouseMotion(int x, int y) {
    if (isDragging) {
        float deltaX = (float)(x - lastMouseX);
        float deltaY = (float)(y - lastMouseY);
        
        lastMouseX = x;
        lastMouseY = y;
        
        game->onMouseDrag(deltaX, deltaY);
    }
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    game->onResize(w, h);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("CS302N_Game");

    glewInit(); // Initialize GLEW before Game

    game = new Game(800, 600);
    game->initialize();

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    
    // Register the new mouse callbacks
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}