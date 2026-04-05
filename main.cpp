#include <GL/glew.h>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#include "game.h"
Game* game = nullptr;

void display() {
    game->render();
    glutSwapBuffers();
}

void idle() {
    game->update(0.016f); // Approx 60 FPS delta time
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // ESC
        delete game;
        exit(0);
    }
    game->onKeyPress(key);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    game->onResize(w, h);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("CS302N_Game");

    glewInit(); // Initialize GLEW before Game

    game = new Game(800, 600);
    game->initialize();

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}