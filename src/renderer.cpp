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
Renderer::Renderer() : mainShader(nullptr), nightMode(false) {}
Renderer::~Renderer()
{
    delete mainShader;
    mainShader = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Night-mode helpers
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::setNightMode(bool night)
{
    nightMode = night;
}

// Activate shader and push the u_nightMode uniform.
// Call this instead of mainShader->use() in every draw that uses the shader.
void Renderer::applyShader()
{
    if (!mainShader) return;
    mainShader->use();
    mainShader->setInt("texture1",  0);
    mainShader->setInt("u_nightMode", nightMode ? 1 : 0);
}

// Darkens a solid colour to match what the fragment shader does for textures.
// Same multiplier as vec3(0.22, 0.26, 0.42) in the shader.
glm::vec3 Renderer::applyNightTint(glm::vec3 c) const
{
    if (!nightMode) return c;
    return c * glm::vec3(0.22f, 0.26f, 0.42f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Initialization
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::initialize()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    std::string ad = std::string(ASSET_DIR);
    mainShader = new Shader((ad + "shaders/vertex.glsl").c_str(),
                            (ad + "shaders/fragment.glsl").c_str());

    loadTexture((ad + "textures/grass.png").c_str(), "grass");
    loadTexture((ad + "textures/grass2.png").c_str(), "grass2");
    loadTexture((ad + "textures/road.png").c_str(), "road");
    loadTexture((ad + "textures/chicken.png").c_str(), "chicken");
    loadTexture((ad + "textures/car.png").c_str(), "car");
    loadTexture((ad + "textures/rail.png").c_str(), "rail");
    loadTexture((ad + "textures/river.png").c_str(), "river");
    loadTexture((ad + "textures/train.png").c_str(), "train");
    loadTexture((ad + "textures/log.png").c_str(), "log");

    // Day sky; night sky is set dynamically in prepareFrame()
    glClearColor(0.29f, 0.59f, 0.86f, 1.0f);
}

void Renderer::prepareFrame()
{
    if (nightMode)
        glClearColor(0.04f, 0.04f, 0.12f, 1.0f);   // deep-night sky
    else
        glClearColor(0.29f, 0.59f, 0.86f, 1.0f);    // daytime sky
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Texture loading
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::loadTexture(const char *path, const std::string &name)
{
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true);
    int w, h, ch;
    unsigned char *data = stbi_load(path, &w, &h, &ch, 4);
    if (data)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        textures[name] = id;
    }
    else
    {
        std::cerr << "Failed to load texture: " << path << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawCube  (solid colour, no texture)
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color)
{
    glm::vec3 c = applyNightTint(color);
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef(scale.x, scale.y, scale.z);
    glColor3f(c.r, c.g, c.b);
    glutSolidCube(1.0f);
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawCubeEmissive  (solid colour, no night tint — for headlights etc.)
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawCubeEmissive(glm::vec3 position, glm::vec3 scale, glm::vec3 color)
{
    // Intentionally does NOT call applyNightTint — emissive surfaces are
    // always drawn at the supplied colour regardless of night mode.
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
//  drawHeadlightBeam  (semi-transparent tapered cone, only useful at night)
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawHeadlightBeam(glm::vec3 origin, float dirX,
                                 float spreadZ, float spreadY, float length)
{
    if (!nightMode) return;   // no-op in daytime

    glDisable(GL_TEXTURE_2D);
    if (mainShader) glUseProgram(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    const float xTip = origin.x;
    const float xFar = origin.x + dirX * length;
    const float y    = origin.y;
    const float z    = origin.z;
    // warm yellow-white beam colour
    const float R = 1.00f, G = 0.95f, B = 0.70f;
    const float alphaTip = 0.22f, alphaFar = 0.0f;

    // ── Horizontal fan (top-down) ──────────────────────────────────────────
    glBegin(GL_TRIANGLES);
      glColor4f(R, G, B, alphaTip);  glVertex3f(xTip, y, z);
      glColor4f(R, G, B, alphaFar);  glVertex3f(xFar, y, z - spreadZ);
      glColor4f(R, G, B, alphaFar);  glVertex3f(xFar, y, z + spreadZ);
    glEnd();

    // ── Vertical fan (side view) ───────────────────────────────────────────
    glBegin(GL_TRIANGLES);
      glColor4f(R, G, B, alphaTip);  glVertex3f(xTip, y,               z);
      glColor4f(R, G, B, alphaFar);  glVertex3f(xFar, y + spreadY,     z);
      glColor4f(R, G, B, alphaFar);  glVertex3f(xFar, y - spreadY * 0.4f, z);
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}


void Renderer::drawSprite(glm::vec3 position, glm::vec3 scale,
                          const std::string &textureName, float rotationY)
{
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationY, 0, 1, 0);
    glScalef(scale.x, scale.y, scale.z);

    if (mainShader)
    {
        applyShader();
    }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures.at(textureName));
    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(-0.5f, -0.5f, 0.05f);
    glTexCoord2f(1, 0);
    glVertex3f(0.5f, -0.5f, 0.05f);
    glTexCoord2f(1, 1);
    glVertex3f(0.5f, 0.5f, 0.05f);
    glTexCoord2f(0, 1);
    glVertex3f(-0.5f, 0.5f, 0.05f);
    glEnd();
    glBegin(GL_QUADS);
    glTexCoord2f(1, 0);
    glVertex3f(-0.5f, -0.5f, -0.05f);
    glTexCoord2f(0, 0);
    glVertex3f(0.5f, -0.5f, -0.05f);
    glTexCoord2f(0, 1);
    glVertex3f(0.5f, 0.5f, -0.05f);
    glTexCoord2f(1, 1);
    glVertex3f(-0.5f, 0.5f, -0.05f);
    glEnd();

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    if (mainShader)
        glUseProgram(0);
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawTexturedCube
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawTexturedCube(glm::vec3 position, glm::vec3 scale,
                                const std::string &textureName, float rotationY)
{
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    if (rotationY != 0.0f)
        glRotatef(rotationY, 0, 1, 0);
    glScalef(scale.x, scale.y, scale.z);

    float rx = scale.x, ry = scale.y, rz = scale.z;
    if (textureName == "log")
        rx *= 2.0f;

    if (mainShader)
    {
        applyShader();
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures.at(textureName));
    glColor3f(1, 1, 1);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    glBegin(GL_QUADS);
    // FRONT
    glTexCoord2f(0, 0);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(rx, 0);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(rx, ry);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, ry);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    // BACK
    glTexCoord2f(0, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(rx, 0);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(rx, ry);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0, ry);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    // TOP
    if (textureName == "rail")
    {
        glTexCoord2f(rz, rx);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glTexCoord2f(0, rx);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glTexCoord2f(0, 0);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glTexCoord2f(rz, 0);
        glVertex3f(0.5f, 0.5f, -0.5f);
    }
    else
    {
        glTexCoord2f(0, rz);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glTexCoord2f(0, 0);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glTexCoord2f(rx, 0);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glTexCoord2f(rx, rz);
        glVertex3f(0.5f, 0.5f, -0.5f);
    }
    // BOTTOM
    glTexCoord2f(rx, rz);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(0, rz);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(0, 0);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(rx, 0);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    // RIGHT
    glTexCoord2f(rz, 0);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(rz, ry);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0, ry);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, 0);
    glVertex3f(0.5f, -0.5f, 0.5f);
    // LEFT
    glTexCoord2f(0, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(rz, 0);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(rz, ry);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, ry);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd();

    glDisable(GL_ALPHA_TEST);
    if (mainShader)
        glUseProgram(0);
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawAnimatedWater
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawAnimatedWater(glm::vec3 position, glm::vec3 scale)
{
    float t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;
    glDisable(GL_TEXTURE_2D);
    if (mainShader)
        glUseProgram(0);

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef(scale.x, scale.y, scale.z);
    glm::vec3 wBody = applyNightTint(glm::vec3(0.30f, 0.65f, 0.90f));
    glColor3f(wBody.r, wBody.g, wBody.b);
    glutSolidCube(1.0f);
    glPopMatrix();

    const float SY = position.y + scale.y * 0.5f + 0.012f;
    const float X0 = position.x - scale.x * 0.5f;
    const float X1 = position.x + scale.x * 0.5f;
    const float Z0 = position.z - scale.z * 0.5f;
    const float Z1 = position.z + scale.z * 0.5f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glm::vec3 wSurf = applyNightTint(glm::vec3(0.45f, 0.78f, 0.97f));
    glColor4f(wSurf.r, wSurf.g, wSurf.b, 0.92f);
    glBegin(GL_QUADS);
    glVertex3f(X0, SY, Z0);
    glVertex3f(X1, SY, Z0);
    glVertex3f(X1, SY, Z1);
    glVertex3f(X0, SY, Z1);
    glEnd();

    float scroll = std::fmod(t * 3.0f, scale.x + 8.0f) - 8.0f;
    float bx0 = X0 + scroll, bx1 = bx0 + 8.0f;
    float cx0 = std::max(bx0, X0), cx1 = std::min(bx1, X1);
    if (cx0 < cx1)
    {
        float len = bx1 - bx0;
        float aL = (1.0f - 2.0f * std::abs((cx0 - bx0) / len - 0.5f)) * 0.18f;
        float aR = (1.0f - 2.0f * std::abs((cx1 - bx0) / len - 0.5f)) * 0.18f;
        glBegin(GL_QUADS);
        glColor4f(0.75f, 0.92f, 1.00f, aL);
        glVertex3f(cx0, SY + 0.001f, Z0);
        glColor4f(0.75f, 0.92f, 1.00f, aR);
        glVertex3f(cx1, SY + 0.001f, Z0);
        glColor4f(0.75f, 0.92f, 1.00f, aR);
        glVertex3f(cx1, SY + 0.001f, Z1);
        glColor4f(0.75f, 0.92f, 1.00f, aL);
        glVertex3f(cx0, SY + 0.001f, Z1);
        glEnd();
    }

    const float edgeD = scale.z * 0.10f;
    glColor4f(0.10f, 0.35f, 0.65f, 0.55f);
    glBegin(GL_QUADS);
    glVertex3f(X0, SY + 0.002f, Z0);
    glVertex3f(X1, SY + 0.002f, Z0);
    glVertex3f(X1, SY + 0.002f, Z0 + edgeD);
    glVertex3f(X0, SY + 0.002f, Z0 + edgeD);
    glEnd();
    glBegin(GL_QUADS);
    glVertex3f(X0, SY + 0.002f, Z1 - edgeD);
    glVertex3f(X1, SY + 0.002f, Z1 - edgeD);
    glVertex3f(X1, SY + 0.002f, Z1);
    glVertex3f(X0, SY + 0.002f, Z1);
    glEnd();

    glDepthMask(GL_TRUE);
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
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == 0 && j == 0)
                continue;
            glm::vec3 cp(x0 + i * vx + vx * 0.5f, position.y, z0 + j * vz + vz * 0.5f);
            glm::vec3 color = ((i == 2 || j == 2) && i <= 2 && j <= 2) ? centerColor : edgeColor;
            drawCube(cp, {vx, size.y, vz}, color);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawEgg
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawEgg(int clicks)
{
    float vSize = Config::EGG_SIZE;
    glm::vec3 shell(0.89f, 0.62f, 0.45f);
    for (int y = 0; y < 7; y++)
    {
        for (int x = -2; x <= 2; x++)
        {
            for (int z = -2; z <= 2; z++)
            {
                bool isVoxel = false;
                if (y == 0 && abs(x) <= 1 && abs(z) <= 1)
                    isVoxel = true;
                if ((y >= 1 && y <= 3) && (abs(x) + abs(z) <= 3) && abs(x) <= 2 && abs(z) <= 2)
                    isVoxel = true;
                if (y == 4 && abs(x) <= 1 && abs(z) <= 1)
                    isVoxel = true;
                if (y == 5 && (abs(x) + abs(z) <= 1))
                    isVoxel = true;
                if (y == 6 && x == 0 && z == 0)
                    isVoxel = true;
                if (!isVoxel)
                    continue;
                bool crack = false;
                if (clicks >= 1 && ((y == 3 && x == 2 && z == 0) || (y == 2 && x == 2 && z == 1) || (y == 4 && x == 1 && z == 1)))
                    crack = true;
                if (clicks >= 2 && ((y == 2 && x == -2 && z == 0) || (y == 3 && x == -1 && z == 1) || (y == 1 && x == -1 && z == 2)))
                    crack = true;
                glm::vec3 pos(x * vSize, y * vSize, z * vSize);
                if (crack)
                    drawCube(pos, glm::vec3(vSize * 0.8f), {0.15f, 0.1f, 0.05f});
                else
                    drawCube(pos, glm::vec3(vSize * 0.93f), shell);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawSignalPost
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawSignalPost(glm::vec3 base, bool lightRed, bool lightGreen)
{
    drawCube(base + glm::vec3(0, 0.55f, 0), {0.09f, 1.10f, 0.09f}, {0.40f, 0.40f, 0.42f});
    drawCube(base + glm::vec3(0, 1.20f, 0), {0.34f, 0.52f, 0.22f}, {0.13f, 0.13f, 0.15f});
    glm::vec3 redCol = lightRed ? glm::vec3(1.00f, 0.08f, 0.08f) : glm::vec3(0.28f, 0.04f, 0.04f);
    glm::vec3 greenCol = lightGreen ? glm::vec3(0.10f, 1.00f, 0.12f) : glm::vec3(0.04f, 0.28f, 0.05f);
    drawCube(base + glm::vec3(0, 1.35f, 0), {0.20f, 0.20f, 0.24f}, redCol);
    drawCube(base + glm::vec3(0, 1.04f, 0), {0.20f, 0.20f, 0.24f}, greenCol);
    drawCube(base + glm::vec3(0, 0.86f, 0), {0.56f, 0.09f, 0.09f}, {0.92f, 0.92f, 0.92f});
    drawCube(base + glm::vec3(-0.15f, 0.86f, 0), {0.08f, 0.18f, 0.08f}, {0.92f, 0.92f, 0.92f});
    drawCube(base + glm::vec3(0.15f, 0.86f, 0), {0.08f, 0.18f, 0.08f}, {0.92f, 0.92f, 0.92f});
}

// =============================================================================
//  MOUNTAIN / BOUNDARY RENDERING
// =============================================================================

float Renderer::heightNoise(float z, int col)
{
    int iz = static_cast<int>(std::round(z));
    unsigned int seed = static_cast<unsigned int>(iz * 2654435761u + col * 2246822519u);
    seed ^= (seed >> 16);
    seed *= 0x45d9f3b;
    seed ^= (seed >> 16);
    return static_cast<float>(seed & 0xFFu) / 255.0f * 0.75f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawMountainSection
//
//  OVERLAP FIX — what was wrong and the minimal change made:
//
//  The visual "stacked arches" bug was caused by exactly three geometry pieces
//  whose Z-extent was wider than one lane (1.0 unit):
//
//    1. Backing slab    slabW = tunnelW + BW*2.4 = 2.124  →  now 1.0
//    2. Rock above slab slabW + 0.10              = 2.224  →  now 1.10
//    3. Header band     tunnelW + BW*1.8          = 1.968  →  now 1.0
//
//  Mathematically, the pilasters (at z±0.88), arch ring strips, and keystone
//  do NOT overlap between adjacent lanes because each lane's left-side pieces
//  and the next lane's right-side pieces land at disjoint Z positions.
//  Those three elements are therefore left completely unchanged.
//
//  isPortalFace is accepted as a parameter for API compatibility but is no
//  longer needed to control overlap — all decorative arch geometry is drawn
//  for every slice, exactly as in the original, just with the narrowed widths.
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawMountainSection(float z, LaneType laneType, float logFlowDir, bool isPortalFace)
{
    const float t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 2.0f);

    // ── Palette ───────────────────────────────────────────────────────────────
    const glm::vec3 rockFace(0.18f, 0.15f, 0.12f);
    const glm::vec3 rockDark(0.28f, 0.24f, 0.20f);
    const glm::vec3 rock1(0.44f, 0.39f, 0.34f);
    const glm::vec3 rock2(0.36f, 0.32f, 0.28f);
    const glm::vec3 rock3(0.52f, 0.47f, 0.41f);
    const glm::vec3 mossTint(0.26f, 0.35f, 0.18f);
    const glm::vec3 snow(0.94f, 0.96f, 1.00f);
    const glm::vec3 snowShade(0.74f, 0.80f, 0.90f);
    const glm::vec3 waterDeep(0.18f, 0.50f, 0.86f);
    const glm::vec3 waterMid(0.38f, 0.72f, 0.98f);
    const glm::vec3 foamWhite(0.92f, 0.97f, 1.00f);
    const glm::vec3 tunVoid(0.03f, 0.02f, 0.04f);
    const glm::vec3 archStone(0.62f, 0.55f, 0.44f);
    const glm::vec3 archLight(0.80f, 0.74f, 0.62f);

    const bool isTunnel = (laneType == LANE_ROAD || laneType == LANE_RAIL);
    const bool isWaterfall = (laneType == LANE_RIVER || laneType == LANE_LILYPAD);

    const float tunnelH = (laneType == LANE_RAIL) ? 2.10f : 1.50f;
    // tunnelW must be < (laneSpacing - 2*BW) ≈ 0.74 so the outer arch ring
    // (expR = tunnelW/2 + BW) stays within this lane's own half-width (0.5).
    // The original 1.50 bled ~0.75 units into neighbours → stacked arches.
    const float tunnelW = 0.70f;

    const int NCOLS = 10;
    const float BASE_H[NCOLS] = {2.8f, 4.0f, 5.4f, 6.5f, 7.2f,
                                 6.6f, 5.5f, 4.2f, 3.2f, 2.4f};
    const bool HAS_SNOW[NCOLS] = {false, false, true, true, true,
                                  true, true, false, false, false};

    float H[NCOLS];
    for (int c = 0; c < NCOLS; c++)
        H[c] = BASE_H[c] + heightNoise(z, c) * 0.85f - 0.30f;

    auto drawCol = [&](float x, float h, glm::vec3 col, bool snow_, bool moss_)
    {
        drawCube({x, h * 0.5f, z}, {1.0f, h, 1.0f}, col);
        if (moss_)
        {
            float mh = h * 0.30f;
            drawCube({x, mh * 0.5f, z}, {1.0f, mh, 1.0f},
                     glm::mix(col, mossTint, 0.45f));
        }
        if (snow_)
        {
            drawCube({x, h + 0.14f, z}, {0.90f, 0.28f, 0.90f}, snow);
            drawCube({x, h + 0.36f, z}, {0.65f, 0.22f, 0.65f}, snowShade);
            if (h > 5.5f)
                drawCube({x, h + 0.58f, z}, {0.42f, 0.18f, 0.42f}, snowShade);
        }
    };

    for (int side = -1; side <= 1; side += 2)
    {
        const float wallX = side * Config::MOUNTAIN_WALL_X;
        auto colX = [&](int c)
        { return wallX + side * static_cast<float>(c); };

        // =============================================================
        //  WATERFALL  (river / lilypad lanes) — UNCHANGED
        // =============================================================
        if (isWaterfall)
        {
            bool isSource = (logFlowDir == 0.0f) || (logFlowDir > 0.0f && side == -1) || (logFlowDir < 0.0f && side == +1);

            if (isSource)
            {
                constexpr float SEG_H = 0.38f;
                constexpr float WFALL_BOT = -0.55f;
                constexpr float WF_SPEED = 7.0f;
                constexpr float WF_STEP = 1.50f;

                for (int wc = 0; wc <= 1; wc++)
                {
                    float wfX = colX(wc);
                    float wfW = (wc == 0) ? 1.10f : 0.80f;
                    float spMul = (wc == 0) ? 1.00f : 0.70f;
                    float phOff = (wc == 0) ? 0.0f : 0.85f;
                    float wfTop = H[wc];

                    for (int seg = 0;; seg++)
                    {
                        float cy = WFALL_BOT + seg * SEG_H + SEG_H * 0.5f;
                        if (cy - SEG_H * 0.5f > wfTop)
                            break;
                        float wave = 0.5f + 0.5f * std::cos(
                                                       t * WF_SPEED * spMul + seg * WF_STEP + phOff);
                        float sharp = wave * wave;
                        glm::vec3 col = (sharp < 0.5f)
                                            ? glm::mix(waterDeep, waterMid, sharp * 2.0f)
                                            : glm::mix(waterMid, foamWhite, (sharp - 0.5f) * 2.0f);
                        float shim = 0.03f * std::sin(t * 8.0f + seg * 0.7f + wc);
                        drawCube({wfX + shim, cy, z}, {wfW, SEG_H * 0.90f, 0.92f}, col);
                    }
                }

                drawCube({colX(0), WFALL_BOT + 0.12f, z}, {1.20f, 0.24f, 1.05f}, foamWhite);
                drawCube({colX(1), WFALL_BOT + 0.12f, z}, {0.92f, 0.20f, 0.88f}, foamWhite);
                drawCube({colX(0) - side * 0.30f, WFALL_BOT + 0.20f, z + 0.28f},
                         {0.52f, 0.13f, 0.48f}, foamWhite);
                drawCube({colX(0) + side * 0.20f, WFALL_BOT + 0.18f, z - 0.25f},
                         {0.42f, 0.11f, 0.40f}, foamWhite);

                float mist = 0.38f + 0.32f * std::sin(t * 3.2f);
                float mist2 = 0.30f + 0.28f * std::sin(t * 2.7f + 1.1f);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDepthMask(GL_FALSE);
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
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);

                drawCol(colX(2), H[2] * 0.90f, rockDark, false, true);
                for (int c = 3; c < NCOLS; c++)
                {
                    glm::vec3 col = HAS_SNOW[c] ? snow
                                                : (c % 3 == 0 ? rock3 : c % 2 == 0 ? rock1
                                                                                   : rock2);
                    drawCol(colX(c), H[c], col, HAS_SNOW[c], false);
                }
            }
            else
            {
                // EXIT side: drainage arch — UNCHANGED
                float x0d = colX(0);
                const float DAP = 0.20f;
                const float DVP = 0.07f;
                auto atDF = [&](float d)
                { return x0d + side * d * 0.5f; };
                auto atDA = [&](float d)
                { return x0d + side * (d * 0.5f - DAP); };
                auto atDV = [&](float d)
                { return x0d + side * (d * 0.5f - DVP); };

                const float drainH = 0.70f;
                const float drainW = 1.10f;
                const float springD = drainH * 0.50f;
                const float archRD = drainH - springD;
                const float halfJD = drainW * 0.50f;
                const float borderD = 0.22f;
                const float expRD = archRD + borderD * 0.5f;

                const glm::vec3 drainStone(0.62f, 0.55f, 0.44f);
                const glm::vec3 drainDark(0.40f, 0.35f, 0.28f);
                const glm::vec3 drainCap(0.74f, 0.68f, 0.54f);

                drawCube({atDF(2.0f), (drainH + 0.35f) * 0.5f, z},
                         {2.0f, drainH + 0.35f, drainW + borderD * 3.0f}, drainDark);
                drawCube({atDA(2.0f), springD * 0.5f, z - halfJD - borderD * 0.5f},
                         {2.0f, springD, borderD}, drainStone);
                drawCube({atDA(2.0f), springD * 0.5f, z + halfJD + borderD * 0.5f},
                         {2.0f, springD, borderD}, drainStone);
                drawCube({atDA(2.0f), borderD * 0.25f, z},
                         {2.0f, borderD * 0.5f, drainW + borderD * 2.0f}, drainStone);
                const int NDA = 10;
                for (int ai = 0; ai < NDA; ai++)
                {
                    float h = expRD * (static_cast<float>(ai) + 0.5f) / NDA;
                    float innerHW = halfJD * std::sqrt(std::max(0.0f, 1.0f - (h / archRD) * (h / archRD)));
                    float outerHW = expRD * std::sqrt(std::max(0.0f, 1.0f - (h / expRD) * (h / expRD)));
                    float cy = springD + h;
                    float sh = expRD / NDA + 0.012f;
                    if (innerHW < 0.03f)
                    {
                        if (outerHW > 0.02f)
                            drawCube({atDA(2.0f), cy, z}, {2.0f, sh, outerHW * 2.0f}, drainCap);
                    }
                    else
                    {
                        float sw = std::max(0.03f, outerHW - innerHW);
                        drawCube({atDA(2.0f), cy, z - innerHW - sw * 0.5f}, {2.0f, sh, sw}, drainStone);
                        drawCube({atDA(2.0f), cy, z + innerHW + sw * 0.5f}, {2.0f, sh, sw}, drainStone);
                    }
                }
                drawCube({atDA(2.05f), springD + expRD, z},
                         {2.05f, borderD * 1.3f, borderD * 1.1f}, drainCap);
                drawCube({atDV(2.1f), springD * 0.50f, z},
                         {2.1f, springD, drainW}, tunVoid);
                for (int ai = 0; ai < NDA; ai++)
                {
                    float h = archRD * (static_cast<float>(ai) + 0.5f) / NDA;
                    float sinV = std::sqrt(std::max(0.0f, 1.0f - (h / archRD) * (h / archRD)));
                    float sw = halfJD * 2.0f * sinV;
                    float cy = springD + h;
                    float sh = archRD / NDA + 0.008f;
                    if (sw < 0.04f)
                        continue;
                    drawCube({atDV(2.1f), cy, z}, {2.1f, sh, sw}, tunVoid);
                }
                float glint = 0.45f + 0.35f * std::sin(t * 4.2f + z * 0.8f);
                glm::vec3 waterG(0.28f * glint, 0.60f * glint, 0.90f * glint);
                drawCube({atDA(0.05f), 0.06f, z}, {0.05f, 0.07f, drainW * 0.85f}, waterG);
                float aboveD = H[0] - drainH - 0.35f;
                if (aboveD > 0.05f)
                {
                    drawCube({atDF(2.0f), drainH + 0.35f + aboveD * 0.5f, z},
                             {2.0f, aboveD, drainW + borderD * 3.0f + 0.1f}, rockFace);
                }
                drawCol(colX(1), H[1], rockDark, false, true);
                drawCol(colX(2), H[2] * 0.90f, rockDark, false, true);
                for (int c = 3; c < NCOLS; c++)
                {
                    glm::vec3 col = HAS_SNOW[c] ? snow
                                                : (c % 3 == 0 ? rock3 : c % 2 == 0 ? rock1
                                                                                   : rock2);
                    drawCol(colX(c), H[c], col, HAS_SNOW[c], false);
                }
            }
            continue;
        }

        // =============================================================
        //  TUNNEL PORTAL  (road / rail lanes)
        //
        //  THE ONLY CHANGES vs the original are the three width values
        //  marked  ← FIX  below.  Everything else is identical.
        //
        //  Why those three? They were the only geometry wider than one
        //  lane (1.0 unit in Z).  The pilasters, arch ring, and keystone
        //  are analytically proven to not overlap between adjacent lanes,
        //  so they are untouched.
        // =============================================================
        if (isTunnel)
        {
            const float x0 = colX(0);
            const float h0 = H[0];

            const float halfJ = tunnelW * 0.5f; // half opening width (Z)
            const float archR = halfJ;          // semicircle radius = half-width
            const float sprH = tunnelH - archR; // spring-line height
            const float BW = 0.26f;             // stone border thickness
            const float expR = archR + BW;      // outer arch radius
            const float rimH = std::max(0.08f, std::min(0.48f, h0 - tunnelH - 0.05f));
            const float totH = tunnelH + rimH;

            // ── FIX 1 of 3 ───────────────────────────────────────────────────
            // ORIGINAL: const float slabW = tunnelW + BW * 2.4f;  // = 2.124
            // That made every slice's backing slab 2.124 units wide in Z,
            // causing ~1.1 units of overlap with the slab of each adjacent slice.
            const float slabW = 1.0f; // exactly one lane width → zero slab overlap
            // ─────────────────────────────────────────────────────────────────

            const float AP = 0.22f;
            const float VP = 0.09f;
            auto atXF = [&](float d)
            { return x0 + side * d * 0.5f; };
            auto atX = [&](float d)
            { return x0 + side * (d * 0.5f - AP); };
            auto atXV = [&](float d)
            { return x0 + side * (d * 0.5f - VP); };

            const glm::vec3 stMain(0.70f, 0.62f, 0.50f);
            const glm::vec3 stDark(0.50f, 0.44f, 0.34f);
            const glm::vec3 stLight(0.84f, 0.77f, 0.62f);

            // ── 1. Backing slab ───────────────────────────────────────────────
            drawCube({atXF(1.8f), totH * 0.5f, z},
                     {1.8f, totH, slabW}, stDark); // slabW now = 1.0  ← FIX 1

            // ── 2. Rock above portal ──────────────────────────────────────────
            float aboveH = h0 - totH;
            if (aboveH > 0.05f)
                drawCube({atXF(1.8f), totH + aboveH * 0.5f, z},
                         {1.8f, aboveH, 1.0f}, stMain); // clamped to lane width

            // ── 3. Jamb pilasters — UNCHANGED (do NOT overlap adj. lanes) ────
            float pZL = z - halfJ - BW * 0.5f;
            float pZR = z + halfJ + BW * 0.5f;
            drawCube({atX(1.9f), sprH * 0.5f, pZL}, {1.9f, sprH, BW}, stMain);
            drawCube({atX(1.9f), sprH * 0.5f, pZR}, {1.9f, sprH, BW}, stMain);
            drawCube({atX(1.95f), BW * 0.25f, pZL}, {1.95f, BW * 0.5f, BW * 1.25f}, stLight);
            drawCube({atX(1.95f), BW * 0.25f, pZR}, {1.95f, BW * 0.5f, BW * 1.25f}, stLight);

            // ── 4. Semicircular arch ring — UNCHANGED (no adj-lane overlap) ──
            const int NA = 10;
            const float shA = expR / NA + 0.014f;
            for (int ai = 0; ai < NA; ai++)
            {
                float h = expR * (static_cast<float>(ai) + 0.5f) / NA;
                float iHW = archR * std::sqrt(std::max(0.0f, 1.0f - (h / archR) * (h / archR)));
                float oHW = expR * std::sqrt(std::max(0.0f, 1.0f - (h / expR) * (h / expR)));
                float cy = sprH + h;
                glm::vec3 vc = (ai % 2 == 0) ? stMain : stDark;
                if (iHW < 0.04f)
                {
                    if (oHW > 0.02f)
                        drawCube({atX(1.9f), cy, z}, {1.9f, shA, oHW * 2.0f}, stLight);
                }
                else
                {
                    float sw = std::max(0.025f, oHW - iHW);
                    drawCube({atX(1.9f), cy, z - iHW - sw * 0.5f}, {1.9f, shA, sw}, vc);
                    drawCube({atX(1.9f), cy, z + iHW + sw * 0.5f}, {1.9f, shA, sw}, vc);
                }
            }
            // Keystone — UNCHANGED
            drawCube({atX(2.0f), sprH + archR + BW * 0.55f, z},
                     {2.0f, BW * 0.9f, BW * 0.85f}, stLight);

            // ── 5. Header band ────────────────────────────────────────────────
            drawCube({atX(1.9f), totH - rimH * 0.5f, z},
                     // ── FIX 3 of 3 ──────────────────────────────────────────
                     // ORIGINAL: {1.9f, rimH, tunnelW + BW * 1.8f}  (= 1.968)
                     {1.9f, rimH, 1.0f}, stMain); // ← FIX 3  (one lane width)

            // ── 6. Tunnel void ────────────────────────────────────────────────
            // Disable polygon offset so the void wins the depth test over the
            // road surface (which has no offset). Without this, the 2/2 offset
            // pushes the void away and lets white lane markings bleed through.
            // Also extend void 0.1 below grade (was flush at y=0, z-fighting).
            glDisable(GL_POLYGON_OFFSET_FILL);
            drawCube({atXV(2.0f), (sprH + 0.1f) * 0.5f - 0.05f, z},
                     {2.0f, sprH + 0.1f, tunnelW}, tunVoid);
            const int NV = 12;
            const float shV = archR / NV + 0.009f;
            for (int ai = 0; ai < NV; ai++)
            {
                float h = archR * (static_cast<float>(ai) + 0.5f) / NV;
                float hw = halfJ * std::sqrt(std::max(0.0f, 1.0f - (h / archR) * (h / archR)));
                float cy = sprH + h;
                float sw = hw * 2.0f + 0.01f;
                if (sw < 0.04f)
                    continue;
                drawCube({atXV(2.0f), cy, z}, {2.0f, shV, sw}, tunVoid);
            }
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(2.0f, 2.0f);

            // ── 7. Interior depth at col 1 ────────────────────────────────────
            drawCol(colX(1), H[1], rockDark, false, false);
            float iW = tunnelW * 0.76f;
            drawCube({colX(1), sprH * 0.5f, z},
                     {1.09f, sprH, iW}, glm::mix(rockDark, tunVoid, 0.5f));
            drawCube({colX(1), sprH * 0.5f, z},
                     {1.11f, sprH, iW * 0.84f}, tunVoid);

            // ── 8. Per-lane accent ────────────────────────────────────────────
            if (laneType == LANE_RAIL)
            {
                float flk = 0.88f + 0.12f * std::sin(t * 44.0f + z);
                glm::vec3 lc = glm::vec3(0.98f, 0.88f, 0.35f) * flk;
                drawCube({atX(1.95f), tunnelH * 0.70f, pZL}, {1.95f, 0.12f, 0.12f}, lc);
                drawCube({atX(1.95f), tunnelH * 0.70f, pZR}, {1.95f, 0.12f, 0.12f}, lc);
            }

            // ── 9. Outer mountain columns ─────────────────────────────────────
            drawCol(colX(2), H[2] * 0.92f, rockDark, false, false);
            for (int c = 3; c < NCOLS; c++)
            {
                glm::vec3 col = HAS_SNOW[c] ? snow
                                            : (c % 3 == 0 ? rock3 : c % 2 == 0 ? rock1
                                                                               : rock2);
                drawCol(colX(c), H[c], col, HAS_SNOW[c], false);
            }
            continue;
        }

        // =============================================================
        //  SOLID MOUNTAIN  (grass lanes and default) — UNCHANGED
        // =============================================================
        for (int c = 0; c < NCOLS; c++)
        {
            float x = colX(c);
            float h = H[c];

            glm::vec3 col;
            if (c == 0)
                col = rockFace;
            else if (c == 1)
                col = rockDark;
            else if (HAS_SNOW[c])
                col = snow;
            else
                col = (c % 3 == 0 ? rock3 : c % 2 == 0 ? rock1
                                                       : rock2);

            bool hasMoss = (c <= 1);
            drawCol(x, h, col, HAS_SNOW[c], hasMoss);
        }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawFoam — UNCHANGED
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawFoam(glm::vec3 position, float width, float depth)
{
    const float t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;

    const glm::vec3 foamBright(0.94f, 0.97f, 1.00f);
    const glm::vec3 foamBlue(0.72f, 0.86f, 0.97f);
    const glm::vec3 foamMid(0.84f, 0.93f, 0.99f);

    drawCube(position, {width, 0.09f, depth}, foamBright);

    for (int i = 0; i < 6; i++)
    {
        float ang = static_cast<float>(i) * 1.047f + t * (i % 2 == 0 ? 1.8f : -1.4f);
        float r = 0.22f + 0.12f * static_cast<float>(i % 3) / 3.0f;
        float cx = position.x + std::cos(ang) * r * width * 0.48f;
        float cz = position.z + std::sin(ang) * r * depth * 0.48f;
        float cs = 0.10f + 0.06f * static_cast<float>(i % 4) / 4.0f;
        float cy = position.y + 0.05f + 0.03f * std::sin(t * 5.0f + i * 1.2f);

        glm::vec3 fc = (i % 3 == 0) ? foamBright : (i % 3 == 1 ? foamBlue : foamMid);
        drawCube({cx, cy, cz}, {cs, cs * 0.55f, cs}, fc);
    }

    float rippleR = 0.5f + 0.18f * std::sin(t * 3.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_TEXTURE_2D);
    float rx0 = position.x - width * 0.5f * rippleR;
    float rx1 = position.x + width * 0.5f * rippleR;
    float rz0 = position.z - depth * 0.5f * rippleR;
    float rz1 = position.z + depth * 0.5f * rippleR;
    float ry = position.y + 0.05f;
    glColor4f(0.88f, 0.95f, 1.0f, 0.45f);
    glBegin(GL_QUADS);
    glVertex3f(rx0, ry, rz0);
    glVertex3f(rx1, ry, rz0);
    glVertex3f(rx1, ry, rz1);
    glVertex3f(rx0, ry, rz1);
    glEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawBackWall — UNCHANGED
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawBackWall()
{
    const glm::vec3 rock1(0.48f, 0.43f, 0.39f);
    const glm::vec3 rock2(0.40f, 0.36f, 0.33f);
    const glm::vec3 darkRk(0.30f, 0.27f, 0.24f);
    const glm::vec3 snow(0.88f, 0.91f, 0.96f);
    const glm::vec3 snowSide(0.80f, 0.84f, 0.90f);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 2.0f);

    const float WALL_Z = 5.5f;

    for (int depth = 0; depth < 3; depth++)
    {
        float dz = WALL_Z + depth * 1.0f;
        for (int xi = -22; xi <= 22; xi++)
        {
            float x = static_cast<float>(xi);
            float noise = heightNoise(dz, xi + 30);
            float h = 2.8f + noise + std::abs(xi) * 0.04f;
            h = std::min(h, 5.8f);
            float hReduced = h - depth * 0.35f;
            if (hReduced <= 0.0f)
                continue;

            bool hasSn = (hReduced > 3.5f);
            glm::vec3 col = (depth == 0) ? darkRk
                                         : (xi % 2 == 0 ? rock1 : rock2);

            drawCube({x, hReduced * 0.5f, dz}, {1.0f, hReduced, 1.0f}, col);
            if (hasSn)
            {
                drawCube({x, hReduced + 0.11f, dz}, {0.78f, 0.22f, 0.78f}, snow);
                drawCube({x, hReduced + 0.22f, dz}, {0.50f, 0.13f, 0.50f}, snowSide);
            }
        }
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Day/Night Cycle & Lighting
// ─────────────────────────────────────────────────────────────────────────────

void Renderer::updateLighting(float currentTime)
{
    // Normalize time to 0-1 over a full day/night cycle
    float cycleTime = std::fmod(currentTime * Config::TIME_SPEED, 1.0f);
    
    // Determine if we're in day or night based on cycle time
    // 0-0.5 = day, 0.5-1.0 = night
    bool isDayTime = cycleTime < 0.5f;
    
    // Calculate smooth transition using smoothstep for blending
    float transitionSpeed = 1.0f / Config::TRANSITION_SMOOTHNESS;
    float blendFactor = 0.0f;
    
    // Smooth transition around 0.5 (dawn/dusk)
    if (cycleTime < 0.5f)
    {
        float distFromMidnight = std::abs(cycleTime - 0.0f);
        float distFromNoon = std::abs(cycleTime - 0.5f);
        blendFactor = 1.0f - (distFromNoon / 0.5f) * 0.5f;
        blendFactor = std::min(1.0f, std::max(0.0f, blendFactor));
    }
    else
    {
        float distFromNoon = std::abs(cycleTime - 0.5f);
        float distFromMidnight = std::abs(cycleTime - 1.0f);
        blendFactor = (distFromNoon / 0.5f) * 0.5f;
        blendFactor = std::min(1.0f, std::max(0.0f, blendFactor));
    }
    
    // Update night mode based on cycle time
    setNightMode(cycleTime >= 0.5f);
}

void Renderer::drawSunAndMoon(float sunProgress, bool isDayTime)
{
    // sunProgress: 0-1 over the cycle, where 0-0.5 = day, 0.5-1.0 = night
    
    // Calculate sun/moon position across the sky
    // Moves from left (-15) to right (+15) in the world, high in the sky
    float sunX = -15.0f + sunProgress * 30.0f;
    float sunY = 8.0f + 3.0f * std::sin(sunProgress * 3.14159f);
    float sunZ = 0.0f;
    
    glPushMatrix();
    glDisable(GL_DEPTH_TEST);
    
    if (isDayTime)
    {
        // Draw sun as a large yellow square
        glm::vec3 sunColor = glm::vec3(1.0f, 0.9f, 0.2f);
        drawCubeEmissive({sunX, sunY, sunZ}, {1.5f, 1.5f, 0.1f}, sunColor);
    }
    else
    {
        // Draw moon as a large pale blue/white square
        glm::vec3 moonColor = glm::vec3(0.85f, 0.9f, 1.0f);
        drawCubeEmissive({sunX, sunY, sunZ}, {1.2f, 1.2f, 0.1f}, moonColor);
    }
    
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
}

float Renderer::getShadowYHeight(LaneType laneType) const
{
    switch (laneType) {
        case LANE_ROAD:
            return Config::SHADOW_HEIGHT_ROAD;
        case LANE_GRASS:
            return Config::SHADOW_HEIGHT_GRASS;
        case LANE_RAIL:
            return Config::SHADOW_HEIGHT_RAIL;
        case LANE_RIVER:
            return Config::SHADOW_HEIGHT_RIVER;
        case LANE_LILYPAD:
            return Config::SHADOW_HEIGHT_LILYPAD;
        default:
            return Config::SHADOW_Y_OFFSET;
    }
}

float Renderer::getShadowFadeFactor(float sunAngle) const
{
    // Calculate fade factor based on sun angle
    // When sun approaches horizon (±π/4 ≈ ±0.7854), shadows fade out
    // Fade range: from SHADOW_FADE_START_ANGLE to π/4 (0.7854)
    const float HORIZON_ANGLE = 3.14159f / 4.0f;  // π/4
    float sunAngleMagnitude = std::abs(sunAngle);
    
    // If angle is less than fade start, full opacity
    if (sunAngleMagnitude < Config::SHADOW_FADE_START_ANGLE) {
        return 1.0f;
    }
    
    // If angle is beyond horizon, no shadow
    if (sunAngleMagnitude >= HORIZON_ANGLE) {
        return 0.0f;
    }
    
    // Linear interpolation between fade start and horizon for smooth transition
    float fadeRange = HORIZON_ANGLE - Config::SHADOW_FADE_START_ANGLE;
    float fadeProgress = (sunAngleMagnitude - Config::SHADOW_FADE_START_ANGLE) / fadeRange;
    
    // Smoothly fade from 1.0 to 0.0
    return 1.0f - std::min(1.0f, fadeProgress);
}

void Renderer::drawShadow(glm::vec3 position, glm::vec3 size, float sunAngle, float sunAngleMagnitude, float shadowFadeFactor, LaneType laneType)
{
    // OPTIMIZED: Render shadow as a simple flat quad instead of cube
    // Fade factor and sun angle magnitude are pre-calculated to avoid redundant calculations
    
    // Skip completely faded shadows
    if (shadowFadeFactor <= 0.0f) return;
    
    // Calculate the shadow length based on sun angle
    float shadowLengthFactor = Config::SHADOW_MIN_LENGTH + 
                              (Config::SHADOW_MAX_LENGTH - Config::SHADOW_MIN_LENGTH) * 
                              std::min(1.0f, sunAngleMagnitude / 1.57f);
    
    float shadowLength = std::max(Config::SHADOW_MIN_LENGTH, size.z * shadowLengthFactor);
    float shadowOffsetX = std::sin(sunAngle) * size.x * 1.5f;
    float shadowOffsetZ = Config::SHADOW_Z_OFFSET;
    
    glm::vec3 shadowPos = position;
    shadowPos.y = getShadowYHeight(laneType) + Config::SHADOW_Y_OFFSET;
    shadowPos.x += shadowOffsetX;
    shadowPos.z += shadowOffsetZ;
    
    float shadowWidth = size.x * 1.2f;
    
    // Render shadow as a flat quad (much faster than glutSolidCube)
    glColor4f(0.0f, 0.0f, 0.0f, Config::SHADOW_OPACITY * shadowFadeFactor);
    
    // Draw quad with 4 vertices (no matrix operations needed)
    glBegin(GL_QUADS);
    float halfW = shadowWidth * 0.5f;
    float z1 = shadowPos.z;
    float z2 = shadowPos.z + shadowLength;
    float x1 = shadowPos.x - halfW;
    float x2 = shadowPos.x + halfW;
    float y = shadowPos.y;
    
    glVertex3f(x1, y, z1);
    glVertex3f(x2, y, z1);
    glVertex3f(x2, y, z2);
    glVertex3f(x1, y, z2);
    glEnd();
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // Reset color
}

void Renderer::drawCharacterShadow(glm::vec3 position, glm::vec3 size, float sunAngle, float sunAngleMagnitude, float shadowFadeFactor, LaneType laneType)
{
    // Render shadow for player character
    glm::vec3 shadowSize = glm::vec3(size.x * 0.9f, size.y * 0.3f, size.z * 0.8f);
    drawShadow(position, shadowSize, sunAngle, sunAngleMagnitude, shadowFadeFactor, laneType);
}

void Renderer::drawObstacleShadow(glm::vec3 position, glm::vec3 size, float sunAngle, float sunAngleMagnitude, float shadowFadeFactor, LaneType laneType)
{
    // Render shadow for obstacles (cars, trains, logs)
    drawShadow(position, size, sunAngle, sunAngleMagnitude, shadowFadeFactor, laneType);
}

void Renderer::drawSignalPostShadow(glm::vec3 basePosition, float sunAngle, float sunAngleMagnitude, float shadowFadeFactor, LaneType laneType)
{
    // Signal posts are tall and thin
    glm::vec3 shadowSize = glm::vec3(0.2f, 1.5f, 0.1f);
    drawShadow(basePosition, shadowSize, sunAngle, sunAngleMagnitude, shadowFadeFactor, laneType);
}