#include "../include/coin.h"
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#include <cmath>

Coin::Coin(glm::vec3 pos) { position = pos; collected = false; }

static void diskFace(float r, int seg) {
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,0,1); glVertex3f(0,0,0);
    for (int i = 0; i <= seg; i++) {
        float a = 2.f * M_PI * i / seg;
        glVertex3f(r*cosf(a), r*sinf(a), 0);
    }
    glEnd();
}

static void coinEdge(float r, float h, int seg) {
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= seg; i++) {
        float a  = 2.f * M_PI * i / seg;
        float rf = r + 0.007f * fabsf(sinf(36*a)); // ridges
        glNormal3f(cosf(a), sinf(a), 0);
        glVertex3f(rf*cosf(a), rf*sinf(a), -h);
        glVertex3f(rf*cosf(a), rf*sinf(a),  h);
    }
    glEnd();
}

static void ring(float r0, float r1, int seg, float z) {
    glBegin(GL_QUAD_STRIP);
    glNormal3f(0,0,1);
    for (int i = 0; i <= seg; i++) {
        float a = 2.f * M_PI * i / seg;
        glVertex3f(r1*cosf(a), r1*sinf(a), z);
        glVertex3f(r0*cosf(a), r0*sinf(a), z);
    }
    glEnd();
}

static void star(float ro, float ri, int pts, float lift) {
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,0,1); glVertex3f(0,0,lift*1.5f);
    for (int i = 0; i <= pts*2; i++) {
        float a = M_PI/pts*i - M_PI/2.f;
        float r = (i%2==0) ? ro : ri;
        glVertex3f(r*cosf(a), r*sinf(a), (i%2==0)?lift:0);
    }
    glEnd();
}

void Coin::render(Renderer& renderer) {
    if (collected) return;
    float t = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT1); glEnable(GL_NORMALIZE);

    GLfloat lp[]={2,4,3,1}, ld[]={1,.9f,.5f,1}, ls[]={1,1,.8f,1};
    glLightfv(GL_LIGHT1,GL_POSITION,lp);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,ld);
    glLightfv(GL_LIGHT1,GL_SPECULAR,ls);

    GLfloat ma[]={.45f,.36f,.08f,1}, md[]={1.f,.82f,.18f,1}, ms[]={1.f,.95f,.55f,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,ma);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,md);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,ms);
    glMaterialf (GL_FRONT_AND_BACK,GL_SHININESS,128);

    const float R=.168f, H=.024f;
    glPushMatrix();
    glTranslatef(position.x, position.y + .05f*sinf(t*2.5f), position.z);
    glRotatef(t*80, 0,1,0);
    glRotatef(18, 1,0,0);

    // Edge
    glColor3f(.90f,.72f,.05f); coinEdge(R, H, 120);

    // Front face
    glPushMatrix(); glTranslatef(0,0,H);
    glColor3f(1.f,.90f,.15f);  diskFace(R, 64);
    glColor3f(.85f,.65f,.02f); ring(R*.87f, R*.96f, 64, .002f); // rim
    glColor3f(1.f,.97f,.50f);  star(R*.40f, R*.17f, 5, .009f);  // emboss
    glPopMatrix();

    // Back face
    glPushMatrix(); glTranslatef(0,0,-H); glRotatef(180,1,0,0);
    glColor3f(1.f,.90f,.15f);  diskFace(R, 64);
    glColor3f(.85f,.65f,.02f); ring(R*.87f, R*.96f, 64, .002f);
    glColor3f(1.f,.97f,.50f);  star(R*.38f, R*.16f, 6, .009f);
    glPopMatrix();

    glPopMatrix();
    glDisable(GL_LIGHT1); glDisable(GL_LIGHTING); glDisable(GL_NORMALIZE);
}

glm::vec3 Coin::getPosition() const { return position; }
float     Coin::getSize()     const { return 0.3f; }