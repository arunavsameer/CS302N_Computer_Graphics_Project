#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <string>
#include <map>
#include "shader.h"
#include "types.h"   // for LaneType

class Renderer {
public:
    Renderer();
    ~Renderer();

    void initialize();
    void prepareFrame();   // clears buffers; respects nightMode for sky colour

    void loadTexture(const char* path, const std::string& name);

    void drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color);

    // Like drawCube but bypasses the night-mode tint — use for emissive
    // geometry such as headlights that should always appear at full brightness.
    void drawCubeEmissive(glm::vec3 position, glm::vec3 scale, glm::vec3 color);

    // Draws a semi-transparent light-cone fan in front of a headlight pair.
    // origin   : midpoint between the two headlight lenses (world space)
    // dirX     : +1 or -1 (vehicle travel direction along X)
    // spreadZ  : half-width of the beam at its far end
    // spreadY  : half-height of the beam at its far end
    // length   : how far the cone reaches along X
    // Only call when isNightMode() == true (it is a no-op otherwise but
    // gating it avoids wasting draw calls during the day).
    void drawHeadlightBeam(glm::vec3 origin, float dirX,
                           float spreadZ, float spreadY, float length);

    void drawTexturedCube(glm::vec3 position, glm::vec3 scale,
                          const std::string& textureName, float rotationY = 0.0f);

    void drawSprite(glm::vec3 position, glm::vec3 scale,
                    const std::string& textureName, float rotationY = 0.0f);

    void drawAnimatedWater(glm::vec3 position, glm::vec3 scale);

    void drawLilypad(glm::vec3 position, glm::vec3 size,
                     glm::vec3 centerColor, glm::vec3 edgeColor);

    void drawEgg(int clicks);

    void drawSignalPost(glm::vec3 base, bool lightRed, bool lightGreen);

    // ── World boundary / environment ─────────────────────────────────────────
    void drawMountainSection(float z, LaneType laneType, float logFlowDir = 0.0f,
                             bool isPortalFace = true);

    void drawFoam(glm::vec3 position, float width, float depth);

    void drawBackWall();

    // ── Night mode ───────────────────────────────────────────────────────────
    // Toggle night mode on/off.  The renderer changes the sky clear-colour,
    // passes u_nightMode to the shader, and darkens solid-colour drawCube calls.
    void setNightMode(bool night);
    bool isNightMode() const { return nightMode; }

private:
    std::map<std::string, unsigned int> textures;
    Shader* mainShader;

    bool nightMode;   // false = day (default), true = night

    // Applies the shader and sets u_nightMode.  Call before any draw that
    // uses mainShader so the uniform is always up-to-date.
    void applyShader();

    // Multiplies a colour by the night-mode ambient factor so solid cubes
    // (drawCube) are also darkened consistently with textured geometry.
    glm::vec3 applyNightTint(glm::vec3 color) const;

    // Shared height-noise helper used by mountain functions.
    static float heightNoise(float z, int col);
};

#endif