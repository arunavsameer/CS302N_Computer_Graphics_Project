#include <GL/glew.h>

// Platform-specific GLUT headers
#ifdef __APPLE__
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <GL/glut.h>
#else
    #include <GL/glut.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include <cstring>

// ============================================================
// Global Variables (Required for GLUT)
// ============================================================

int g_windowWidth = 1280;
int g_windowHeight = 720;
bool g_keys[256];
bool g_special_keys[256];
float g_lastTime = 0.0f;
float g_deltaTime = 0.0f;

// ============================================================
// GLUT Callbacks
// ============================================================

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)g_windowWidth / (float)g_windowHeight, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    
    // TEST: Draw colorful triangle
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);  // Red
    glVertex3f(0.0f, 0.5f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);  // Green
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue
    glVertex3f(0.5f, -0.5f, 0.0f);
    glEnd();
    
    glutSwapBuffers();
}

void reshapeCallback(int width, int height) {
    if (height == 0) height = 1;
    g_windowWidth = width;
    g_windowHeight = height;
    glViewport(0, 0, width, height);
}

void keyboardCallback(unsigned char key, int x, int y) {
    g_keys[key] = true;
    if (key == 27) {  // ESC key
        std::cout << "Exiting game..." << std::endl;
        exit(0);
    }
    std::cout << "Keyboard: " << key << std::endl;
}

void keyboardUpCallback(unsigned char key, int x, int y) {
    g_keys[key] = false;
}

void specialKeyCallback(int key, int x, int y) {
    g_special_keys[key] = true;
    std::cout << "Special Key: " << key << std::endl;
}

void specialKeyUpCallback(int key, int x, int y) {
    g_special_keys[key] = false;
}

void timerCallback(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0);  // ~60 FPS
}

// ============================================================
// Main Function
// ============================================================

int main(int argc, char** argv) {
    std::cout << "================================================" << std::endl;
    std::cout << "  CS302N - Crossy Road Game (Cross-Platform)" << std::endl;
    std::cout << "================================================" << std::endl;
    
    // GLUT initialization
    std::cout << "\n[*] Initializing GLUT..." << std::endl;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(g_windowWidth, g_windowHeight);
    glutInitWindowPosition(100, 100);
    
    int window = glutCreateWindow("CS302N - Crossy Road Game");
    
    if (window < 1) {
        std::cerr << "[ERROR] Failed to create GLUT window!" << std::endl;
        return -1;
    }
    
    std::cout << "[OK] GLUT window created!" << std::endl;
    
    // GLEW initialization
    std::cout << "[*] Initializing GLEW..." << std::endl;
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    
    if (err != GLEW_OK) {
        std::cerr << "[ERROR] GLEW failed: " << glewGetErrorString(err) << std::endl;
        return -1;
    }
    
    std::cout << "[OK] GLEW initialized!" << std::endl;
    
    // Print OpenGL info
    std::cout << "\n[*] OpenGL Information:" << std::endl;
    std::cout << "    Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "    GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "    Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // Register callbacks
    std::cout << "\n[*] Registering callbacks..." << std::endl;
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialKeyCallback);
    glutSpecialUpFunc(specialKeyUpCallback);
    glutTimerFunc(16, timerCallback, 0);
    
    // OpenGL settings
    std::cout << "[*] Configuring OpenGL..." << std::endl;
    glViewport(0, 0, g_windowWidth, g_windowHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    
    std::memset(g_keys, false, sizeof(g_keys));
    std::memset(g_special_keys, false, sizeof(g_special_keys));
    
    std::cout << "[OK] All systems ready!\n" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Arrow Keys / W/A/S/D - Movement" << std::endl;
    std::cout << "  Space - Jump" << std::endl;
    std::cout << "  C - Camera" << std::endl;
    std::cout << "  M - Day/Night" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "You should see a colorful triangle in the center" << std::endl;
    std::cout << "================================================\n" << std::endl;
    
    g_lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glutMainLoop();
    
    std::cout << "[*] Shutting down..." << std::endl;
    std::cout << "[OK] Game closed successfully!" << std::endl;
    
    return 0;
}