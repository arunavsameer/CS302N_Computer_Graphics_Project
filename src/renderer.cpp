#include "../include/renderer.h"
#include "../include/stb_image.h"
#include "../include/types.h"
#include <GL/glew.h>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#include <iostream>
#include <cmath>
#include <algorithm>

#ifndef ASSET_DIR
#define ASSET_DIR "../../assets/"
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  Construction / Destruction
// ─────────────────────────────────────────────────────────────────────────────
Renderer::Renderer()  { mainShader = nullptr; }
Renderer::~Renderer() { delete mainShader; mainShader = nullptr; }

// ─────────────────────────────────────────────────────────────────────────────
//  Initialization
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::initialize() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    std::string ad = std::string(ASSET_DIR);
    mainShader = new Shader((ad + "shaders/vertex.glsl").c_str(),
                            (ad + "shaders/fragment.glsl").c_str());

    loadTexture((ad + "textures/grass.png").c_str(),  "grass");
    loadTexture((ad + "textures/grass2.png").c_str(), "grass2");
    loadTexture((ad + "textures/road.png").c_str(),   "road");
    loadTexture((ad + "textures/chicken.png").c_str(),"chicken");
    loadTexture((ad + "textures/car.png").c_str(),    "car");
    loadTexture((ad + "textures/rail.png").c_str(),   "rail");
    loadTexture((ad + "textures/river.png").c_str(),  "river");
    loadTexture((ad + "textures/train.png").c_str(),  "train");
    loadTexture((ad + "textures/log.png").c_str(),    "log");

    glClearColor(0.29f, 0.59f, 0.86f, 1.0f);
}

void Renderer::prepareFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Texture loading
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::loadTexture(const char* path, const std::string& name) {
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true);
    int w, h, ch;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);
    if (data) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        textures[name] = id;
    } else {
        std::cerr << "Failed to load texture: " << path << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawCube  (solid colour, no texture)
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef(scale.x, scale.y, scale.z);
    glColor3f(color.r, color.g, color.b);
    glutSolidCube(1.0f);
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawSprite
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawSprite(glm::vec3 position, glm::vec3 scale,
                           const std::string& textureName, float rotationY)
{
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationY, 0, 1, 0);
    glScalef(scale.x, scale.y, scale.z);

    if (mainShader) { mainShader->use(); mainShader->setInt("texture1", 0); }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures.at(textureName));
    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex3f(-0.5f,-0.5f, 0.05f);
        glTexCoord2f(1,0); glVertex3f( 0.5f,-0.5f, 0.05f);
        glTexCoord2f(1,1); glVertex3f( 0.5f, 0.5f, 0.05f);
        glTexCoord2f(0,1); glVertex3f(-0.5f, 0.5f, 0.05f);
    glEnd();
    glBegin(GL_QUADS);
        glTexCoord2f(1,0); glVertex3f(-0.5f,-0.5f,-0.05f);
        glTexCoord2f(0,0); glVertex3f( 0.5f,-0.5f,-0.05f);
        glTexCoord2f(0,1); glVertex3f( 0.5f, 0.5f,-0.05f);
        glTexCoord2f(1,1); glVertex3f(-0.5f, 0.5f,-0.05f);
    glEnd();

    glDisable(GL_BLEND);
    if (mainShader) glUseProgram(0);
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawTexturedCube
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawTexturedCube(glm::vec3 position, glm::vec3 scale,
                                  const std::string& textureName, float rotationY)
{
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    if (rotationY != 0.0f) glRotatef(rotationY, 0, 1, 0);
    glScalef(scale.x, scale.y, scale.z);

    float rx = scale.x, ry = scale.y, rz = scale.z;
    if (textureName == "log") rx *= 2.0f;

    if (mainShader) { mainShader->use(); mainShader->setInt("texture1", 0); }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures.at(textureName));
    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
        // FRONT
        glTexCoord2f(0,0); glVertex3f(-0.5f,-0.5f, 0.5f);
        glTexCoord2f(rx,0); glVertex3f(0.5f,-0.5f, 0.5f);
        glTexCoord2f(rx,ry); glVertex3f(0.5f, 0.5f, 0.5f);
        glTexCoord2f(0,ry); glVertex3f(-0.5f, 0.5f, 0.5f);
        // BACK
        glTexCoord2f(0,0); glVertex3f(-0.5f,-0.5f,-0.5f);
        glTexCoord2f(rx,0); glVertex3f(0.5f,-0.5f,-0.5f);
        glTexCoord2f(rx,ry); glVertex3f(0.5f, 0.5f,-0.5f);
        glTexCoord2f(0,ry); glVertex3f(-0.5f, 0.5f,-0.5f);
        // TOP
        if (textureName == "rail") {
            glTexCoord2f(rz,rx); glVertex3f(-0.5f, 0.5f,-0.5f);
            glTexCoord2f(0, rx); glVertex3f(-0.5f, 0.5f, 0.5f);
            glTexCoord2f(0, 0);  glVertex3f( 0.5f, 0.5f, 0.5f);
            glTexCoord2f(rz,0);  glVertex3f( 0.5f, 0.5f,-0.5f);
        } else {
            glTexCoord2f(0, rz); glVertex3f(-0.5f, 0.5f,-0.5f);
            glTexCoord2f(0, 0);  glVertex3f(-0.5f, 0.5f, 0.5f);
            glTexCoord2f(rx,0);  glVertex3f( 0.5f, 0.5f, 0.5f);
            glTexCoord2f(rx,rz); glVertex3f( 0.5f, 0.5f,-0.5f);
        }
        // BOTTOM
        glTexCoord2f(rx,rz); glVertex3f(-0.5f,-0.5f,-0.5f);
        glTexCoord2f(0, rz); glVertex3f( 0.5f,-0.5f,-0.5f);
        glTexCoord2f(0, 0);  glVertex3f( 0.5f,-0.5f, 0.5f);
        glTexCoord2f(rx,0);  glVertex3f(-0.5f,-0.5f, 0.5f);
        // RIGHT
        glTexCoord2f(rz,0);  glVertex3f( 0.5f,-0.5f,-0.5f);
        glTexCoord2f(rz,ry); glVertex3f( 0.5f, 0.5f,-0.5f);
        glTexCoord2f(0, ry); glVertex3f( 0.5f, 0.5f, 0.5f);
        glTexCoord2f(0, 0);  glVertex3f( 0.5f,-0.5f, 0.5f);
        // LEFT
        glTexCoord2f(0, 0);  glVertex3f(-0.5f,-0.5f,-0.5f);
        glTexCoord2f(rz,0);  glVertex3f(-0.5f,-0.5f, 0.5f);
        glTexCoord2f(rz,ry); glVertex3f(-0.5f, 0.5f, 0.5f);
        glTexCoord2f(0, ry); glVertex3f(-0.5f, 0.5f,-0.5f);
    glEnd();

    if (mainShader) glUseProgram(0);
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawAnimatedWater
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawAnimatedWater(glm::vec3 position, glm::vec3 scale) {
    float t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;
    glDisable(GL_TEXTURE_2D);
    if (mainShader) glUseProgram(0);

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef(scale.x, scale.y, scale.z);
    glColor3f(0.30f, 0.65f, 0.90f);
    glutSolidCube(1.0f);
    glPopMatrix();

    const float SY = position.y + scale.y * 0.5f + 0.003f;
    const float X0 = position.x - scale.x * 0.5f;
    const float X1 = position.x + scale.x * 0.5f;
    const float Z0 = position.z - scale.z * 0.5f;
    const float Z1 = position.z + scale.z * 0.5f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.45f, 0.78f, 0.97f, 0.92f);
    glBegin(GL_QUADS);
        glVertex3f(X0,SY,Z0); glVertex3f(X1,SY,Z0);
        glVertex3f(X1,SY,Z1); glVertex3f(X0,SY,Z1);
    glEnd();

    float scroll = std::fmod(t * 3.0f, scale.x + 8.0f) - 8.0f;
    float bx0 = X0 + scroll, bx1 = bx0 + 8.0f;
    float cx0  = std::max(bx0, X0), cx1 = std::min(bx1, X1);
    if (cx0 < cx1) {
        float len = bx1 - bx0;
        float aL  = (1.0f - 2.0f * std::abs((cx0 - bx0) / len - 0.5f)) * 0.18f;
        float aR  = (1.0f - 2.0f * std::abs((cx1 - bx0) / len - 0.5f)) * 0.18f;
        glBegin(GL_QUADS);
            glColor4f(0.75f,0.92f,1.00f,aL); glVertex3f(cx0,SY+0.001f,Z0);
            glColor4f(0.75f,0.92f,1.00f,aR); glVertex3f(cx1,SY+0.001f,Z0);
            glColor4f(0.75f,0.92f,1.00f,aR); glVertex3f(cx1,SY+0.001f,Z1);
            glColor4f(0.75f,0.92f,1.00f,aL); glVertex3f(cx0,SY+0.001f,Z1);
        glEnd();
    }

    const float edgeD = scale.z * 0.10f;
    glColor4f(0.10f, 0.35f, 0.65f, 0.55f);
    glBegin(GL_QUADS);
        glVertex3f(X0,SY+0.002f,Z0);       glVertex3f(X1,SY+0.002f,Z0);
        glVertex3f(X1,SY+0.002f,Z0+edgeD); glVertex3f(X0,SY+0.002f,Z0+edgeD);
    glEnd();
    glBegin(GL_QUADS);
        glVertex3f(X0,SY+0.002f,Z1-edgeD); glVertex3f(X1,SY+0.002f,Z1-edgeD);
        glVertex3f(X1,SY+0.002f,Z1);       glVertex3f(X0,SY+0.002f,Z1);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawLilypad
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawLilypad(glm::vec3 position, glm::vec3 size,
                             glm::vec3 centerColor, glm::vec3 edgeColor)
{
    float vx = size.x / 4.0f, vz = size.z / 4.0f;
    float x0 = position.x - size.x * 0.5f;
    float z0 = position.z - size.z * 0.5f;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == 0 && j == 0) continue;
            glm::vec3 cp(x0 + i*vx + vx*0.5f, position.y, z0 + j*vz + vz*0.5f);
            glm::vec3 color = ((i==2 || j==2) && i<=2 && j<=2) ? centerColor : edgeColor;
            drawCube(cp, {vx, size.y, vz}, color);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawEgg
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawEgg(int clicks) {
    float vSize = Config::EGG_SIZE;
    glm::vec3 shell(0.89f, 0.62f, 0.45f);
    for (int y = 0; y < 7; y++) {
        for (int x = -2; x <= 2; x++) {
            for (int z = -2; z <= 2; z++) {
                bool isVoxel = false;
                if (y==0 && abs(x)<=1 && abs(z)<=1) isVoxel=true;
                if ((y>=1&&y<=3) && (abs(x)+abs(z)<=3) && abs(x)<=2 && abs(z)<=2) isVoxel=true;
                if (y==4 && abs(x)<=1 && abs(z)<=1) isVoxel=true;
                if (y==5 && (abs(x)+abs(z)<=1)) isVoxel=true;
                if (y==6 && x==0 && z==0) isVoxel=true;
                if (!isVoxel) continue;
                bool crack = false;
                if (clicks>=1 && ((y==3&&x==2&&z==0)||(y==2&&x==2&&z==1)||(y==4&&x==1&&z==1))) crack=true;
                if (clicks>=2 && ((y==2&&x==-2&&z==0)||(y==3&&x==-1&&z==1)||(y==1&&x==-1&&z==2))) crack=true;
                glm::vec3 pos(x*vSize, y*vSize, z*vSize);
                if (crack) drawCube(pos, glm::vec3(vSize*0.8f), {0.15f,0.1f,0.05f});
                else        drawCube(pos, glm::vec3(vSize*0.93f), shell);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawSignalPost
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawSignalPost(glm::vec3 base, bool lightRed, bool lightGreen) {
    drawCube(base+glm::vec3(0,0.55f,0), {0.09f,1.10f,0.09f}, {0.40f,0.40f,0.42f});
    drawCube(base+glm::vec3(0,1.20f,0), {0.34f,0.52f,0.22f}, {0.13f,0.13f,0.15f});
    glm::vec3 redCol   = lightRed   ? glm::vec3(1.00f,0.08f,0.08f) : glm::vec3(0.28f,0.04f,0.04f);
    glm::vec3 greenCol = lightGreen ? glm::vec3(0.10f,1.00f,0.12f) : glm::vec3(0.04f,0.28f,0.05f);
    drawCube(base+glm::vec3(0,1.35f,0), {0.20f,0.20f,0.24f}, redCol);
    drawCube(base+glm::vec3(0,1.04f,0), {0.20f,0.20f,0.24f}, greenCol);
    drawCube(base+glm::vec3(0,0.86f,0), {0.56f,0.09f,0.09f}, {0.92f,0.92f,0.92f});
    drawCube(base+glm::vec3(-0.15f,0.86f,0), {0.08f,0.18f,0.08f}, {0.92f,0.92f,0.92f});
    drawCube(base+glm::vec3( 0.15f,0.86f,0), {0.08f,0.18f,0.08f}, {0.92f,0.92f,0.92f});
}

// =============================================================================
//  MOUNTAIN / BOUNDARY RENDERING
// =============================================================================

// Deterministic height noise: returns a stable value in [0, 0.8]
// based purely on z-slice index and column index (not time-dependent).
float Renderer::heightNoise(float z, int col) {
    // Round z to nearest integer to get a consistent seed per lane-Z
    int iz = static_cast<int>(std::round(z));
    unsigned int seed = static_cast<unsigned int>(iz * 2654435761u + col * 2246822519u);
    seed ^= (seed >> 16);
    seed *= 0x45d9f3b;
    seed ^= (seed >> 16);
    return static_cast<float>(seed & 0xFFu) / 255.0f * 0.75f; // 0 .. 0.75
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawMountainSection
//
//  Renders ONE Z-slice of the left and right mountain walls.
//  The caller iterates over every visible Z position and calls this once per Z.
//
//  Visual rules per lane type:
//    LANE_ROAD / LANE_RAIL  → rectangular tunnel opening at the cliff face
//    LANE_RIVER / LANE_LILYPAD → animated waterfall cascade; mountain peaks
//                                still visible beyond the gap
//    LANE_GRASS (default)   → solid rock wall with snow caps on tall columns
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawMountainSection(float z, LaneType laneType, float logFlowDir, bool isPortalFace) {
    const float t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;

    // ── Palette ───────────────────────────────────────────────────────────────
    const glm::vec3 rockFace  (0.18f, 0.15f, 0.12f);  // darkest inner cliff
    const glm::vec3 rockDark  (0.28f, 0.24f, 0.20f);  // dark cliff rock
    const glm::vec3 rock1     (0.44f, 0.39f, 0.34f);  // warm mid rock
    const glm::vec3 rock2     (0.36f, 0.32f, 0.28f);  // cool mid rock
    const glm::vec3 rock3     (0.52f, 0.47f, 0.41f);  // lighter highlight rock
    const glm::vec3 mossTint  (0.26f, 0.35f, 0.18f);  // moss on lower cliff
    const glm::vec3 snow      (0.94f, 0.96f, 1.00f);  // bright snow
    const glm::vec3 snowShade (0.74f, 0.80f, 0.90f);  // shaded snow face
    const glm::vec3 waterDeep (0.18f, 0.50f, 0.86f);  // dark deep water (base)
    const glm::vec3 waterMid  (0.38f, 0.72f, 0.98f);  // mid water
    const glm::vec3 foamWhite (0.92f, 0.97f, 1.00f);  // bright foam crest
    const glm::vec3 tunVoid   (0.03f, 0.02f, 0.04f);  // tunnel void (near black)
    const glm::vec3 archStone (0.62f, 0.55f, 0.44f);  // arch stone frame — warm sandstone
    const glm::vec3 archLight (0.80f, 0.74f, 0.62f);  // arch highlight stone — light cream

    const bool isTunnel   = (laneType == LANE_ROAD || laneType == LANE_RAIL);
    const bool isWaterfall= (laneType == LANE_RIVER || laneType == LANE_LILYPAD);

    // Tunnel heights — must comfortably clear the tallest vehicle.
    // Road: truck cab tops out at ~1.2 units above ground → use 1.5 for headroom.
    // Rail: locomotive stack reaches ~1.8 units → use 2.1 for clearance.
    const float tunnelH = (laneType == LANE_RAIL) ? 2.10f : 1.50f;
    // Tunnel opening width in X — wide enough for the widest vehicle (truck ~1.0Z,
    // but the opening is in the X-face of the column so just needs to look right).
    const float tunnelW = 1.50f;

    // ── Mountain column profile (dramatic silhouette) ─────────────────────────
    // 10 columns per side extending outward from the innermost cliff column.
    // Heights rise steeply (col 0→4) then taper (4→9) like a natural ridge.
    const int   NCOLS   = 10;
    const float BASE_H[NCOLS]   = { 2.8f, 4.0f, 5.4f, 6.5f, 7.2f,
                                     6.6f, 5.5f, 4.2f, 3.2f, 2.4f };
    const bool  HAS_SNOW[NCOLS] = { false, false, true, true, true,
                                     true,  true, false, false, false };

    float H[NCOLS];
    for (int c = 0; c < NCOLS; c++)
        H[c] = BASE_H[c] + heightNoise(z, c) * 0.85f - 0.30f;

    // ── Small helper: draw one rock column + optional snow/moss ──────────────
    auto drawCol = [&](float x, float h, glm::vec3 col, bool snow_, bool moss_) {
        drawCube({x, h * 0.5f, z}, {1.0f, h, 1.0f}, col);
        if (moss_) {
            float mh = h * 0.30f;
            drawCube({x, mh * 0.5f, z}, {1.0f, mh, 1.0f},
                     glm::mix(col, mossTint, 0.45f));
        }
        if (snow_) {
            // 3-layer tapered snow cap
            drawCube({x, h + 0.14f, z}, {0.90f, 0.28f, 0.90f}, snow);
            drawCube({x, h + 0.36f, z}, {0.65f, 0.22f, 0.65f}, snowShade);
            if (h > 5.5f)
                drawCube({x, h + 0.58f, z}, {0.42f, 0.18f, 0.42f}, snowShade);
        }
    };

    // ── Per-side rendering (side=-1 → left wall, side=+1 → right wall) ───────
    for (int side = -1; side <= 1; side += 2) {
        const float wallX = side * Config::MOUNTAIN_WALL_X;
        // colX(c): innermost col (c=0) is AT the wall, higher c goes outward
        auto colX = [&](int c) { return wallX + side * static_cast<float>(c); };

        // =============================================================
        //  WATERFALL  (river / lilypad lanes)
        //
        //  Only the SOURCE side (where logs originate) gets a waterfall.
        //  logFlowDir > 0  →  logs move +X  →  LEFT wall  (side=-1) is source
        //  logFlowDir < 0  →  logs move -X  →  RIGHT wall (side=+1) is source
        //  logFlowDir == 0 →  both sides get waterfall (fallback)
        //
        //  The EXIT side (where logs disappear) shows a dark cliff face
        //  with a low stone arch at river level where the water flows under.
        // =============================================================
        if (isWaterfall) {
            // Determine if this side is the waterfall source
            bool isSource = (logFlowDir == 0.0f)
                          || (logFlowDir > 0.0f && side == -1)
                          || (logFlowDir < 0.0f && side == +1);

            if (isSource) {
                // ── SOURCE SIDE: full animated waterfall cascade ──────────────
                constexpr int   WF_SEGS   = 16;
                constexpr float SEG_H     = 0.38f;
                constexpr float WFALL_BOT = -0.55f;
                constexpr float WF_SPEED  =  7.0f;
                constexpr float WF_STEP   =  1.50f;

                for (int wc = 0; wc <= 1; wc++) {
                    float wfX   = colX(wc);
                    float wfW   = (wc == 0) ? 1.10f : 0.80f;
                    float spMul = (wc == 0) ? 1.00f : 0.70f;
                    float phOff = (wc == 0) ? 0.0f  : 0.85f;

                    for (int seg = 0; seg < WF_SEGS; seg++) {
                        float cy    = WFALL_BOT + seg * SEG_H + SEG_H * 0.5f;
                        float wave  = 0.5f + 0.5f * std::cos(
                            t * WF_SPEED * spMul + seg * WF_STEP + phOff);
                        float sharp = wave * wave;
                        glm::vec3 col = (sharp < 0.5f)
                            ? glm::mix(waterDeep, waterMid,  sharp * 2.0f)
                            : glm::mix(waterMid,  foamWhite, (sharp - 0.5f) * 2.0f);
                        float shim = 0.03f * std::sin(t * 8.0f + seg * 0.7f + wc);
                        drawCube({wfX + shim, cy, z}, {wfW, SEG_H * 0.90f, 0.92f}, col);
                    }
                }

                // Foam splash at falls base
                drawCube({colX(0), WFALL_BOT + 0.12f, z}, {1.20f, 0.24f, 1.05f}, foamWhite);
                drawCube({colX(1), WFALL_BOT + 0.12f, z}, {0.92f, 0.20f, 0.88f}, foamWhite);
                drawCube({colX(0) - side * 0.30f, WFALL_BOT + 0.20f, z + 0.28f},
                         {0.52f, 0.13f, 0.48f}, foamWhite);
                drawCube({colX(0) + side * 0.20f, WFALL_BOT + 0.18f, z - 0.25f},
                         {0.42f, 0.11f, 0.40f}, foamWhite);

                // Mist rising from base
                float mist  = 0.38f + 0.32f * std::sin(t * 3.2f);
                float mist2 = 0.30f + 0.28f * std::sin(t * 2.7f + 1.1f);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(0.86f, 0.94f, 1.0f, 0.18f * mist);
                glPushMatrix();
                glTranslatef(colX(0), WFALL_BOT + 0.5f, z);
                glScalef(2.0f, 0.9f, 1.4f);
                glutSolidCube(1.0f);
                glPopMatrix();
                glColor4f(0.86f, 0.94f, 1.0f, 0.13f * mist2);
                glPushMatrix();
                glTranslatef(colX(0) + side * 0.4f, WFALL_BOT + 0.8f, z + 0.3f);
                glScalef(1.3f, 0.7f, 1.1f);
                glutSolidCube(1.0f);
                glPopMatrix();
                glDisable(GL_BLEND);

                drawCol(colX(2), H[2] * 0.90f, rockDark, false, true);
                for (int c = 3; c < NCOLS; c++) {
                    glm::vec3 col = HAS_SNOW[c] ? snow
                        : (c % 3 == 0 ? rock3 : c % 2 == 0 ? rock1 : rock2);
                    drawCol(colX(c), H[c], col, HAS_SNOW[c], false);
                }

            } else {
                // ── EXIT SIDE: drainage arch where the river flows under the mountain ──
                //
                //  Small stone arch at water level — the river disappears through
                //  this hole.  Animated water glint and foam ripple visible inside.
                //  The rest of the mountain face is solid rock.

                float x0d = colX(0);
                // Helper: near face at x0d, depth into mountain
                auto atD = [&](float d) { return x0d + side * d * 0.5f; };

                const float drainH  = 0.70f;   // arch opening height
                const float drainW  = 1.10f;   // arch opening width (Z)
                const float springD = drainH * 0.50f;
                const float archRD  = drainH - springD;
                const float halfJD  = drainW * 0.50f;
                const float borderD = 0.22f;
                const float expRD   = archRD + borderD * 0.5f;

                const glm::vec3 drainStone(0.38f, 0.34f, 0.30f);
                const glm::vec3 drainDark (0.22f, 0.19f, 0.17f);
                const glm::vec3 drainCap  (0.50f, 0.45f, 0.38f);

                // ── Backing slab ─────────────────────────────────────────────
                drawCube({atD(2.0f), (drainH + 0.35f) * 0.5f, z},
                         {2.0f, drainH + 0.35f, drainW + borderD * 3.0f}, drainDark);

                // ── Stone arch frame ─────────────────────────────────────────
                // Jamb bars
                drawCube({atD(2.4f), springD * 0.5f, z - halfJD - borderD * 0.5f},
                         {2.4f, springD, borderD}, drainStone);
                drawCube({atD(2.4f), springD * 0.5f, z + halfJD + borderD * 0.5f},
                         {2.4f, springD, borderD}, drainStone);
                // Ground sill
                drawCube({atD(2.4f), borderD * 0.25f, z},
                         {2.4f, borderD * 0.5f, drainW + borderD * 2.0f}, drainStone);
                // Arch crown strips
                const int NDA = 12;
                for (int ai = 0; ai < NDA; ai++) {
                    float tM      = (static_cast<float>(ai) + 0.5f) / NDA;
                    float sinV    = std::sqrt(std::max(0.0f, 1.0f - tM * tM));
                    float outerHW = (halfJD + borderD) * sinV;
                    float innerHW = halfJD * sinV;
                    float cy      = springD + tM * expRD;
                    float sh      = expRD / NDA + 0.012f;
                    if (innerHW < 0.03f) {
                        drawCube({atD(2.4f), cy, z}, {2.4f, sh, outerHW * 2.0f + 0.02f}, drainStone);
                    } else {
                        float sw = std::max(0.03f, outerHW - innerHW);
                        drawCube({atD(2.4f), cy, z - innerHW - sw * 0.5f}, {2.4f, sh, sw}, drainStone);
                        drawCube({atD(2.4f), cy, z + innerHW + sw * 0.5f}, {2.4f, sh, sw}, drainStone);
                    }
                }
                // Cap stone
                drawCube({atD(2.5f), springD + expRD, z},
                         {2.5f, borderD * 1.4f, borderD * 1.2f}, drainCap);

                // ── Drainage void (dark hole) ─────────────────────────────────
                drawCube({atD(2.6f), springD * 0.50f, z},
                         {2.6f, springD, drainW}, tunVoid);
                for (int ai = 0; ai < NDA; ai++) {
                    float tM   = (static_cast<float>(ai) + 0.5f) / NDA;
                    float sinV = std::sqrt(std::max(0.0f, 1.0f - tM * tM));
                    float sw   = halfJD * 2.0f * sinV;
                    float cy   = springD + tM * archRD;
                    float sh   = archRD / NDA + 0.008f;
                    if (sw < 0.04f) continue;
                    drawCube({atD(2.6f), cy, z}, {2.6f, sh, sw}, tunVoid);
                }

                // ── Animated water glint at arch entrance ─────────────────────
                float glint = 0.45f + 0.35f * std::sin(t * 4.2f + z * 0.8f);
                glm::vec3 waterG(0.28f * glint, 0.60f * glint, 0.90f * glint);
                drawCube({atD(0.10f), 0.06f, z}, {0.10f, 0.07f, drainW * 0.85f}, waterG);

                // ── Rock above drainage arch ──────────────────────────────────
                float aboveD = H[0] - drainH - 0.35f;
                if (aboveD > 0.05f) {
                    drawCube({atD(2.0f), drainH + 0.35f + aboveD * 0.5f, z},
                             {2.0f, aboveD, drainW + borderD * 3.0f + 0.1f}, rockFace);
                }

                // ── Outer mountain columns ────────────────────────────────────
                drawCol(colX(2), H[2] * 0.90f, rockDark, false, true);
                for (int c = 3; c < NCOLS; c++) {
                    glm::vec3 col = HAS_SNOW[c] ? snow
                        : (c % 3 == 0 ? rock3 : c % 2 == 0 ? rock1 : rock2);
                    drawCol(colX(c), H[c], col, HAS_SNOW[c], false);
                }
            }
            continue;
        }

        // =============================================================
        //  IMPROVED TUNNEL PORTAL  (road / rail lanes)
        //
        //  Architecture (viewed from play area, facing the cliff):
        //
        //   ┌─[K1]─[K2]─[K3]─[K4]─[K5]─┐  ← alternating keystone crown
        //   │ cap ╔══════════════╗ cap   │
        //   │ [P] ║              ║ [P]   │  ← pilasters with cap/base
        //   │ [P] ║  STEPPED     ║ [P]   │
        //   │ [P] ║  ARCH VOID   ║ [P]   │
        //   │ base╚══════════════╝ base  │
        //   ─────────────────────────────
        //
        //  The void itself is rendered as 3 stacked slabs (wide centre +
        //  narrower sides) so the upper corners are filled with stone,
        //  creating a vaulted-arch silhouette without bezier math.
        // =============================================================
        if (isTunnel) {
            // ── ORIENTATION NOTE ──────────────────────────────────────────────
            //  The mountain walls sit at ±X.  Vehicles travel along X, so the
            //  tunnel portal face must lie in the Y-Z plane (visible when looking
            //  along X).  All "width" dimensions are therefore in Z and all
            //  "depth-into-mountain" dimensions are in X.
            float x0   = colX(0);
            float h0   = H[0];
            float pW   = 0.46f;   // pilaster width (Z) — wider for presence
            float rimH = 0.58f;   // crown height above opening

            // ── INTERIOR SLICES (multi-cell tunnel) ───────────────────────────
            //  Only the tunnel void is punched through plain rock.
            //  This prevents the full decorative arch from repeating on every
            //  Z slice when a ROAD/RAIL block spans more than one lane cell.
            if (!isPortalFace) {
                // Col 0: full-height rock face with void punched through
                drawCube({x0, h0 * 0.5f, z}, {1.0f, h0, 1.0f}, rockFace);

                const float springH  = tunnelH * 0.55f;
                const float archRad  = tunnelH - springH;
                const float halfJamb = tunnelW * 0.50f;
                const int   NF       = 16;

                // Rectangular jamb void
                drawCube({x0, springH * 0.50f, z}, {1.30f, springH, tunnelW}, tunVoid);
                // Semicircular arch head void
                for (int ai = 0; ai < NF; ai++) {
                    float tM   = (static_cast<float>(ai) + 0.5f) / NF;
                    float sinV = std::sqrt(std::max(0.0f, 1.0f - tM * tM));
                    float sw   = halfJamb * 2.0f * sinV;
                    float cy   = springH + tM * archRad;
                    float sh   = archRad / NF + 0.009f;
                    if (sw < 0.05f) continue;
                    drawCube({x0, cy, z}, {1.30f, sh, sw}, tunVoid);
                }
                // Dark inner ring at col 1 — simulates depth inside the tunnel
                drawCube({colX(1), h0 * 0.5f, z}, {1.0f, h0, 1.0f}, rockDark);
                drawCube({colX(1), springH * 0.50f, z},
                         {1.12f, springH, tunnelW * 0.80f},
                         glm::mix(rockDark, tunVoid, 0.5f));

                // Cols 2+ normal mountain
                drawCol(colX(2), H[2] * 0.92f, rockDark, false, false);
                for (int c = 3; c < NCOLS; c++) {
                    glm::vec3 col = HAS_SNOW[c] ? snow
                        : (c % 3 == 0 ? rock3 : c % 2 == 0 ? rock1 : rock2);
                    drawCol(colX(c), H[c], col, HAS_SNOW[c], false);
                }
                continue;
            }

            // ── Colours ──────────────────────────────────────────────────────
            glm::vec3 frameCol  = glm::mix(archStone, rockFace, 0.25f);
            glm::vec3 capCol    = archLight;
            glm::vec3 keyCol    = glm::vec3(archLight.r + 0.07f,
                                            archLight.g + 0.06f,
                                            archLight.b + 0.05f);
            glm::vec3 innerRing = glm::mix(rockDark, tunVoid, 0.38f);

            // ── atX helper ───────────────────────────────────────────────────
            //  Returns the X centre for a slab of given X-depth whose NEAR FACE
            //  sits exactly at x0 (the mountain inner wall, visible to player).
            //  For the right wall (side=+1) the slab grows outward (+X).
            //  For the left wall (side=-1) the slab grows outward (−X).
            auto atX = [&](float depth) { return x0 + side * depth * 0.5f; };

            // ── 1. FULL BACKING SLAB ──────────────────────────────────────────
            // Near face at x0; 2.0 depth gives a thick visible face from 3/4 angle
            drawCube({atX(2.0f), (tunnelH + rimH) * 0.5f, z},
                     {2.0f, tunnelH + rimH + 0.14f, tunnelW + pW * 2.5f},
                     frameCol);

            // ── 2. KEYSTONE CROWN  (5 alternating courses) ───────────────────
            const int   NK      = 5;
            const float crownW  = tunnelW + pW * 2.5f;
            const float kBlockW = crownW / NK;
            for (int k = 0; k < NK; k++) {
                float kz = (z - crownW * 0.5f) + (k + 0.5f) * kBlockW;
                glm::vec3 kc = (k % 2 == 0) ? capCol : archStone;
                drawCube({atX(2.2f), tunnelH + rimH * 0.5f, kz},
                         {2.2f, rimH, kBlockW * 0.92f}, kc);
            }
            // Central keystone stands proud of its neighbours
            drawCube({atX(2.3f), tunnelH + rimH * 0.64f, z},
                     {2.3f, rimH * 0.76f, kBlockW * 0.82f}, keyCol);

            // ── 3. SIDE PILASTERS ─────────────────────────────────────────────
            // Pilasters are positioned along Z (left/right of the lane opening)
            float pLeftZ  = z - tunnelW * 0.5f - pW * 0.5f;
            float pRightZ = z + tunnelW * 0.5f + pW * 0.5f;
            // Shafts
            drawCube({atX(2.0f), tunnelH * 0.5f, pLeftZ},  {2.0f, tunnelH, pW}, archStone);
            drawCube({atX(2.0f), tunnelH * 0.5f, pRightZ}, {2.0f, tunnelH, pW}, archStone);
            // Capitals (top)
            drawCube({atX(2.1f), tunnelH + 0.09f, pLeftZ},  {2.1f, 0.18f, pW + 0.12f}, capCol);
            drawCube({atX(2.1f), tunnelH + 0.09f, pRightZ}, {2.1f, 0.18f, pW + 0.12f}, capCol);
            // Bases (bottom)
            drawCube({atX(2.1f), 0.09f, pLeftZ},  {2.1f, 0.18f, pW + 0.12f}, capCol);
            drawCube({atX(2.1f), 0.09f, pRightZ}, {2.1f, 0.18f, pW + 0.12f}, capCol);
            // Horizontal entablature band at mid-shaft
            drawCube({atX(2.05f), tunnelH * 0.50f, pLeftZ},  {2.05f, 0.08f, pW + 0.07f}, capCol);
            drawCube({atX(2.05f), tunnelH * 0.50f, pRightZ}, {2.05f, 0.08f, pW + 0.07f}, capCol);

            // ── 4. ARCH FRAME — stone border ring, NO interior fill ────────────
            //
            //  Portal face is in Y-Z plane.  "Left/right" of the arch opening
            //  are ±Z offsets from lane centre.  X scale is the frame thickness
            //  (depth protruding from the mountain face).
            //
            //  Frame depth 2.4 > pilasters (2.0) > slab (2.0) — protrudes toward player.

            const float border   = 0.32f;
            const float springH  = tunnelH * 0.55f;
            const float archRad  = tunnelH - springH;
            const float halfJamb = tunnelW * 0.50f;
            const float expR     = archRad + border * 0.5f;

            const glm::vec3 frameB(0.70f, 0.62f, 0.50f);  // sandstone arch border
            const glm::vec3 kStone(0.94f, 0.88f, 0.72f);  // bright cream keystone

            // Vertical jamb bars (Z-positioned, X depth)
            drawCube({atX(2.4f), springH * 0.5f, z - halfJamb - border * 0.5f},
                     {2.4f, springH, border}, frameB);
            drawCube({atX(2.4f), springH * 0.5f, z + halfJamb + border * 0.5f},
                     {2.4f, springH, border}, frameB);
            // Ground sill
            drawCube({atX(2.4f), border * 0.25f, z},
                     {2.4f, border * 0.5f, tunnelW + border * 2.0f}, frameB);

            // Semicircular arch crown: left + right strips only (20 levels)
            // Alternating light/dark voussoir wedge stones for classic arch look
            const int NF = 20;
            const glm::vec3 vouA(0.78f, 0.70f, 0.56f);  // lighter voussoir
            const glm::vec3 vouB(0.54f, 0.48f, 0.38f);  // darker voussoir
            for (int ai = 0; ai < NF; ai++) {
                float tM      = (static_cast<float>(ai) + 0.5f) / NF;
                float sinV    = std::sqrt(std::max(0.0f, 1.0f - tM * tM));
                float outerHW = (halfJamb + border) * sinV;
                float innerHW = halfJamb * sinV;
                float cy      = springH + tM * expR;
                float sh      = expR / NF + 0.018f;
                glm::vec3 vc  = (ai % 2 == 0) ? vouA : vouB;

                if (innerHW < 0.05f) {
                    drawCube({atX(2.4f), cy, z}, {2.4f, sh, outerHW * 2.0f + 0.02f}, vc);
                } else {
                    float sw = std::max(0.04f, outerHW - innerHW);
                    drawCube({atX(2.4f), cy, z - innerHW - sw * 0.5f}, {2.4f, sh, sw}, vc);
                    drawCube({atX(2.4f), cy, z + innerHW + sw * 0.5f}, {2.4f, sh, sw}, vc);
                }
            }

            // Keystone: proud block at crown apex — deepest element, pops forward
            drawCube({atX(2.5f), springH + expR, z},
                     {2.5f, border * 1.6f, border * 1.5f}, kStone);
            // Carved lintel slab above keystone — banner-like header block
            drawCube({atX(2.45f), springH + expR + border * 1.4f, z},
                     {2.45f, border * 0.9f, tunnelW + pW * 1.6f}, archLight);
            // Decorative recessed panel inset on lintel
            drawCube({atX(2.55f), springH + expR + border * 1.4f, z},
                     {2.55f, border * 0.45f, tunnelW * 0.55f},
                     glm::mix(archStone, tunVoid, 0.35f));

            // ── 5. ARCH VOID — punches black through the frame ─────────────────
            //
            //  X-scale 1.28 > frame's 1.24 → near face slightly closer to camera
            //  so depth test overwrites the frame at the arch boundary edge.

            // Rectangular jamb void (2.6 > frame 2.4 → overwrites frame at opening edge)
            drawCube({atX(2.6f), springH * 0.50f, z},
                     {2.6f, springH, tunnelW}, tunVoid);

            // Semicircular arch head void
            for (int ai = 0; ai < NF; ai++) {
                float tM   = (static_cast<float>(ai) + 0.5f) / NF;
                float sinV = std::sqrt(std::max(0.0f, 1.0f - tM * tM));
                float sw   = halfJamb * 2.0f * sinV;
                float cy   = springH + tM * archRad;
                float sh   = archRad / NF + 0.009f;
                if (sw < 0.05f) continue;
                drawCube({atX(2.6f), cy, z}, {2.6f, sh, sw}, tunVoid);
            }

            // Interior depth shadow at col 1 (dark rim creates tunnel depth)
            float iW = tunnelW * 0.78f;
            float iH = springH;
            drawCube({colX(1), iH * 0.50f, z}, {1.09f, iH, iW}, innerRing);
            drawCube({colX(1), iH * 0.50f, z}, {1.11f, iH, iW * 0.86f}, tunVoid);

            // ── 6. Rock above portal ──────────────────────────────────────────
            float aboveH = h0 - tunnelH - rimH;
            if (aboveH > 0.05f) {
                drawCube({atX(2.0f), tunnelH + rimH + aboveH * 0.5f, z},
                         {2.0f, aboveH, tunnelW + pW * 2.5f + 0.08f},
                         rockFace);
            }

            // ── 7. Lane-specific decorations ──────────────────────────────────
            if (laneType == LANE_RAIL) {
                // Flickering lanterns on pilaster shafts (Z-positioned)
                float flicker = 0.85f + 0.15f * std::sin(t * 43.7f);
                glm::vec3 lc  = glm::vec3(0.95f, 0.88f, 0.40f) * flicker;
                drawCube({atX(2.05f), tunnelH * 0.70f, pLeftZ},  {2.05f, 0.14f, 0.14f}, lc);
                drawCube({atX(2.05f), tunnelH * 0.70f, pRightZ}, {2.05f, 0.14f, 0.14f}, lc);
                // Soft glow halos
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(lc.r, lc.g, lc.b, 0.24f * flicker);
                glPushMatrix();
                  glTranslatef(atX(2.05f), tunnelH * 0.70f, pLeftZ);
                  glScalef(0.48f, 0.48f, 0.48f);
                  glutSolidCube(1.0f);
                glPopMatrix();
                glColor4f(lc.r, lc.g, lc.b, 0.24f * flicker);
                glPushMatrix();
                  glTranslatef(atX(2.05f), tunnelH * 0.70f, pRightZ);
                  glScalef(0.48f, 0.48f, 0.48f);
                  glutSolidCube(1.0f);
                glPopMatrix();
                glDisable(GL_BLEND);
                // Hazard stripes on lower pilaster shafts
                glm::vec3 strY(0.92f, 0.76f, 0.08f);
                glm::vec3 strK(0.10f, 0.10f, 0.10f);
                const int   NS   = 4;
                const float strH = tunnelH * 0.32f / NS;
                for (int s = 0; s < NS; s++) {
                    float sy = 0.18f + s * strH * 2.2f;
                    glm::vec3 sc = (s % 2 == 0) ? strY : strK;
                    drawCube({atX(2.1f), sy + strH * 0.5f, pLeftZ},  {2.1f, strH, pW * 0.52f}, sc);
                    drawCube({atX(2.1f), sy + strH * 0.5f, pRightZ}, {2.1f, strH, pW * 0.52f}, sc);
                }
            } else {
                // Road tunnel: worn threshold sill + tyre-mark stripe
                glm::vec3 sill(0.58f, 0.54f, 0.46f);
                glm::vec3 mark(0.20f, 0.18f, 0.16f);
                drawCube({atX(2.2f), 0.04f, z}, {2.2f, 0.08f, tunnelW * 0.90f}, sill);
                drawCube({atX(2.3f), 0.02f, z}, {2.3f, 0.04f, tunnelW * 0.18f}, mark);
                // Wall-mounted torch brackets on pilasters
                float flicker = 0.82f + 0.18f * std::sin(t * 37.3f + z);
                glm::vec3 torchC = glm::vec3(1.00f, 0.70f, 0.20f) * flicker;
                // Bracket arm
                drawCube({atX(2.15f), tunnelH * 0.62f, pLeftZ},  {2.15f, 0.07f, 0.22f}, archStone);
                drawCube({atX(2.15f), tunnelH * 0.62f, pRightZ}, {2.15f, 0.07f, 0.22f}, archStone);
                // Flame core
                drawCube({atX(2.15f), tunnelH * 0.69f, pLeftZ},  {2.15f, 0.13f, 0.13f}, torchC);
                drawCube({atX(2.15f), tunnelH * 0.69f, pRightZ}, {2.15f, 0.13f, 0.13f}, torchC);
                // Glow halo
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(torchC.r, torchC.g, 0.10f, 0.20f * flicker);
                glPushMatrix();
                  glTranslatef(atX(2.15f), tunnelH * 0.69f, pLeftZ);
                  glScalef(0.42f, 0.42f, 0.42f);
                  glutSolidCube(1.0f);
                glPopMatrix();
                glColor4f(torchC.r, torchC.g, 0.10f, 0.20f * flicker);
                glPushMatrix();
                  glTranslatef(atX(2.15f), tunnelH * 0.69f, pRightZ);
                  glScalef(0.42f, 0.42f, 0.42f);
                  glutSolidCube(1.0f);
                glPopMatrix();
                glDisable(GL_BLEND);
            }

            // ── 8. Continue mountain from col 2 outward ───────────────────────
            drawCol(colX(2), H[2] * 0.92f, rockDark, false, false);
            for (int c = 3; c < NCOLS; c++) {
                glm::vec3 col = HAS_SNOW[c] ? snow
                    : (c % 3 == 0 ? rock3 : c % 2 == 0 ? rock1 : rock2);
                drawCol(colX(c), H[c], col, HAS_SNOW[c], false);
            }
            continue;
        }

        // =============================================================
        //  SOLID MOUNTAIN  (grass lanes and default)
        //  Full profile: dark inner face → mid rock → snow-capped peaks
        // =============================================================
        for (int c = 0; c < NCOLS; c++) {
            float x = colX(c);
            float h = H[c];

            glm::vec3 col;
            if      (c == 0)       col = rockFace;
            else if (c == 1)       col = rockDark;
            else if (HAS_SNOW[c])  col = snow;
            else                   col = (c % 3 == 0 ? rock3 : c % 2 == 0 ? rock1 : rock2);

            bool hasMoss = (c <= 1);  // mossy on the two inner (shaded) columns
            drawCol(x, h, col, HAS_SNOW[c], hasMoss);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawFoam
//
//  Animated white foam patch placed at the river mouth where it meets the
//  mountain cliff face.  Called once per river / lilypad lane end (both sides).
//
//  position = world-space centre of the foam slab (Y at water surface)
//  width    = X extent, depth = Z extent
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawFoam(glm::vec3 position, float width, float depth) {
    const float t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;

    const glm::vec3 foamBright(0.94f, 0.97f, 1.00f);
    const glm::vec3 foamBlue  (0.72f, 0.86f, 0.97f);
    const glm::vec3 foamMid   (0.84f, 0.93f, 0.99f);

    // Base foam slab
    drawCube(position, {width, 0.09f, depth}, foamBright);

    // Animated churning foam chunks (6 per call)
    for (int i = 0; i < 6; i++) {
        // Each chunk orbits at its own angular speed
        float ang = static_cast<float>(i) * 1.047f    // 60° apart base
                    + t * (i % 2 == 0 ? 1.8f : -1.4f);
        float r  = 0.22f + 0.12f * static_cast<float>(i % 3) / 3.0f;
        float cx = position.x + std::cos(ang) * r * width  * 0.48f;
        float cz = position.z + std::sin(ang) * r * depth  * 0.48f;
        float cs = 0.10f + 0.06f * static_cast<float>(i % 4) / 4.0f;
        float cy = position.y + 0.05f + 0.03f * std::sin(t * 5.0f + i * 1.2f);

        glm::vec3 fc = (i % 3 == 0) ? foamBright : (i % 3 == 1 ? foamBlue : foamMid);
        drawCube({cx, cy, cz}, {cs, cs * 0.55f, cs}, fc);
    }

    // Extra thin ripple ring (translucent quad)
    float rippleR = 0.5f + 0.18f * std::sin(t * 3.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    float rx0 = position.x - width  * 0.5f * rippleR;
    float rx1 = position.x + width  * 0.5f * rippleR;
    float rz0 = position.z - depth  * 0.5f * rippleR;
    float rz1 = position.z + depth  * 0.5f * rippleR;
    float ry  = position.y + 0.05f;
    glColor4f(0.88f, 0.95f, 1.0f, 0.45f);
    glBegin(GL_QUADS);
        glVertex3f(rx0,ry,rz0); glVertex3f(rx1,ry,rz0);
        glVertex3f(rx1,ry,rz1); glVertex3f(rx0,ry,rz1);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawBackWall
//
//  A mountain ridge drawn at the back of the starting safe zone (positive Z).
//  Spans the full playable X width plus a bit extra.  Height varies
//  deterministically per column so it looks like a natural cliff face.
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawBackWall() {
    const glm::vec3 rock1  (0.48f, 0.43f, 0.39f);
    const glm::vec3 rock2  (0.40f, 0.36f, 0.33f);
    const glm::vec3 darkRk (0.30f, 0.27f, 0.24f);
    const glm::vec3 snow   (0.88f, 0.91f, 0.96f);
    const glm::vec3 snowSide(0.80f, 0.84f, 0.90f);

    // Back wall sits one row behind the last safe-zone lane
    // (startZ for safe zone is ~5 * CELL_SIZE, so we draw at z ≈ 6)
    const float WALL_Z = 5.5f;

    // One row thick, but we draw several depth layers for solidity
    for (int depth = 0; depth < 3; depth++) {
        float dz = WALL_Z + depth * 1.0f;
        for (int xi = -22; xi <= 22; xi++) {
            float x = static_cast<float>(xi);
            float noise = heightNoise(dz, xi + 30); // +30 to avoid same seed as sides
            float h = 2.8f + noise + std::abs(xi) * 0.04f;
            h = std::min(h, 5.8f);
            float hReduced = h - depth * 0.35f;  // back rows slightly shorter
            if (hReduced <= 0.0f) continue;

            bool hasSn = (hReduced > 3.5f);
            glm::vec3 col = (depth == 0) ? darkRk
                           : (xi % 2 == 0 ? rock1 : rock2);

            drawCube({x, hReduced * 0.5f, dz}, {1.0f, hReduced, 1.0f}, col);
            if (hasSn) {
                drawCube({x, hReduced + 0.11f, dz}, {0.78f, 0.22f, 0.78f}, snow);
                drawCube({x, hReduced + 0.22f, dz}, {0.50f, 0.13f, 0.50f}, snowSide);
            }
        }
    }
}