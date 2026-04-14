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
    void prepareFrame();

    void loadTexture(const char* path, const std::string& name);

    void drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color);

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
    // Draw one Z-slice of the left+right mountain walls.
    // laneType tells us whether to cut a tunnel (ROAD/RAIL),
    // render a waterfall gap (RIVER/LILYPAD), or solid rock (GRASS).
    // logFlowDir: +1 = logs flow right (waterfall on left side),
    //             -1 = logs flow left  (waterfall on right side),
    //              0 = both sides (fallback / non-river lanes)
    // isPortalFace: true  = draw the full decorative arch portal (once per lane entrance)
    //               false = draw only rock + void interior (subsequent Z slices of
    //                       the same multi-cell tunnel lane)
    void drawMountainSection(float z, LaneType laneType, float logFlowDir = 0.0f,
                             bool isPortalFace = true);

    // Animated foam patch used at the river mouth where it meets the cliff.
    // position = world-space centre of the foam slab.
    void drawFoam(glm::vec3 position, float width, float depth);

    // Static back-wall mountain ridge drawn at the starting zone boundary.
    void drawBackWall();

private:
    std::map<std::string, unsigned int> textures;
    Shader* mainShader;

    // Shared height-noise helper used by mountain functions.
    static float heightNoise(float z, int col);
};

#endif