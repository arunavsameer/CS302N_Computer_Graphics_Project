#include "../include/character_dino.h"

std::vector<glm::vec3> CharacterDino::getWaterDeathPalette() const {
    return {{.65f,.42f,.18f},{.65f,.42f,.18f},{.48f,.30f,.12f},
            {.88f,.76f,.54f},{1,.6f,.7f},{1,1,1},{.05f,.05f,.05f}};
}

void CharacterDino::drawModel(Renderer& renderer) const {
    glm::vec3 brn={.65f,.42f,.18f}, brd={.48f,.30f,.12f};
    glm::vec3 brm={.56f,.36f,.15f}, tan={.88f,.76f,.54f};
    glm::vec3 spc={.38f,.24f,.08f}, wht={1,1,1};
    glm::vec3 blk={.05f,.05f,.05f}, pnk={1,.6f,.7f};
    glm::vec3 amb={.90f,.60f,.10f};

    // Hips/lower body
    renderer.drawCube({0,.44f,-.12f},{.70f,.28f,.62f},brn);
    renderer.drawCube({0,.32f,-.02f},{.52f,.14f,.60f},tan); // belly
    renderer.drawCube({0,.58f,-.18f},{.58f,.14f,.50f},brm); // hip ridge

    // Hind legs: thigh, shin, foot, 3 toe claws per side
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.28f,.34f,-.16f},{.24f,.26f,.26f},brn);
        renderer.drawCube({s*.28f,.16f,-.08f},{.18f,.20f,.22f},brd);
        renderer.drawCube({s*.28f,.05f,.04f}, {.20f,.10f,.28f},brd);
        for(float t:{-.06f,0.f,.06f})
            renderer.drawCube({s*.28f+t,.04f,.26f},{.05f,.05f,.06f},wht);
    }

    // Tail (4 tapered segments)
    renderer.drawCube({0,.48f,-.52f}, {.44f,.28f,.28f},brn);
    renderer.drawCube({0,.44f,-.76f}, {.34f,.22f,.24f},brm);
    renderer.drawCube({0,.40f,-.96f}, {.24f,.16f,.22f},brd);
    renderer.drawCube({0,.36f,-1.12f},{.14f,.10f,.18f},brd);

    // Dorsal spines (loop over {y, z, height})
    static const float sp[][3]={
        {.80f,.05f,.18f},{.72f,-.18f,.16f},{.64f,-.38f,.16f},
        {.58f,-.56f,.14f},{.53f,-.76f,.12f},{.48f,-.95f,.10f}};
    for(auto& s:sp)
        renderer.drawCube({0,s[0],s[1]},{.07f,s[2],.07f},spc);

    // Upper body + chest
    renderer.drawCube({0,.70f,.14f},{.56f,.24f,.38f},brn);
    renderer.drawCube({0,.64f,.18f},{.40f,.20f,.42f},tan);

    // Tiny arms: upper, forearm, 2 claws per side
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.30f,.70f,.32f},{.12f,.10f,.18f},brd);
        renderer.drawCube({s*.30f,.62f,.42f},{.10f,.08f,.14f},brd);
        for(float t:{-.04f,.04f})
            renderer.drawCube({s*.30f+t,.57f,.50f},{.04f,.05f,.06f},wht);
    }

    // Head: skull, snout, jaw, brow ridge
    renderer.drawCube({0,1.06f,.30f},{.56f,.52f,.52f},brn);
    renderer.drawCube({0,.98f,.64f},{.44f,.36f,.38f},brm);
    renderer.drawCube({0,.96f,.88f}, {.36f,.28f,.20f},brm);
    renderer.drawCube({0,.84f,.72f}, {.38f,.16f,.42f},tan);
    renderer.drawCube({0,.82f,.90f}, {.30f,.12f,.18f},tan);
    renderer.drawCube({0,1.20f,.50f},{.48f,.10f,.26f},brd);

    // Eyes (amber iris, slit pupil, shine) – symmetric
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.22f,1.16f,.58f},{.12f,.12f,.04f},amb);
        renderer.drawCube({s*.22f,1.16f,.61f},{.05f,.10f,.03f},blk);
        renderer.drawCube({s*(.22f+.04f),1.20f,.63f},{.03f,.03f,.02f},wht);
    }

    // Upper teeth (4) + lower teeth (2)
    for(float tx:{-.12f,-.04f,.04f,.12f})
        renderer.drawCube({tx,.84f,.96f},{.05f,.08f,.05f},wht);
    for(float s:{-1.f,1.f})
        renderer.drawCube({s*.08f,.86f,.96f},{.04f,.05f,.04f},wht);

    // Tongue
    renderer.drawCube({0,.86f,.96f},{.14f,.04f,.10f},pnk);
    renderer.drawCube({0,.85f,1.02f},{.11f,.04f,.06f},pnk);
}