#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

float angleX = 20.0f, angleY = 30.0f;


void drawCube(float sx, float sy, float sz) {
    glPushMatrix();
    glScalef(sx, sy, sz);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void drawCar() {
    glColor3f(0.2f, 0.5f, 1.0f);
    glPushMatrix();
    glTranslatef(0, 1.0f, 0);
    drawCube(2.5f, 0.85f, 1.5f);
    glPopMatrix();
    
    glColor3f(0.95f, 0.95f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.15f, 1.9f, 0);
    drawCube(1.3f, 0.75f, 1.25f);
    glPopMatrix();
    
    glColor3f(0.1f, 0.1f, 0.15f);
    glPushMatrix();
    glTranslatef(-0.5f, 1.85f, 0.6f);
    drawCube(0.6f, 0.5f, 0.3f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.5f, 1.85f, -0.6f);
    drawCube(0.6f, 0.5f, 0.3f);
    glPopMatrix();
    
    glColor3f(0.05f, 0.05f, 0.08f);
    
    float wheelX[] = {0.85f, 0.85f, -0.85f, -0.85f};
    float wheelZ[] = {0.8f, -0.8f, 0.8f, -0.8f};
    
    for(int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(wheelX[i], 0.35f, wheelZ[i]);
        drawCube(0.65f, 0.65f, 0.5f);
        glPopMatrix();
    }
    
    glColor3f(0.1f, 0.3f, 0.6f);
    
    glPushMatrix();
    glTranslatef(1.3f, 0.9f, 0);
    drawCube(0.3f, 0.5f, 1.5f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-1.3f, 0.9f, 0);
    drawCube(0.3f, 0.5f, 1.5f);
    glPopMatrix();
}

void drawEnvironment() {
    glColor3f(0.58f, 0.88f, 0.35f);
    glPushMatrix();
    glScalef(20, 0.1f, 20);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    gluLookAt(5, 4, 8,   
              0, 1, 0,      
              0, 1, 0);     
    
    glRotatef(angleX, 1, 0, 0);
    glRotatef(angleY, 0, 1, 0);
    
    drawEnvironment();
    drawCar();
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 27: // ESC
            exit(0);
            break;
        case 'w':
        case 'W':
            angleX = fminf(angleX + 3, 85);
            break;
        case 's':
        case 'S':
            angleX = fmaxf(angleX - 3, -85);
            break;
        case 'a':
        case 'A':
            angleY -= 3;
            break;
        case 'd':
        case 'D':
            angleY += 3;
            break;
        case 'r':
        case 'R':
            angleX = 20;
            angleY = 30;
            break;
    }
    glutPostRedisplay();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    glEnable(GL_MULTISAMPLE); 
    
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    glClearColor(0.52f, 0.8f, 1.0f, 1.0f);
    GLfloat light_pos[] = {5, 5, 5, 0};
    GLfloat light_color[] = {1, 1, 1, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    
    // Request a multisampled display mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GL_MULTISAMPLE); 
    
    glutInitWindowSize(800, 600);
    glutCreateWindow("car");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    
    glutMainLoop();
    return 0;
}