#include "../include/character_dog.h"
#include "../include/character.h"
#include <algorithm>

std::vector<glm::vec3> CharacterDog::getWaterDeathPalette() const {
    return {{.80f,.60f,.35f},{.98f,.95f,.85f},{.10f,.10f,.10f},{1,.6f,.7f}};
}

void CharacterDog::drawModel(Renderer& renderer) const {
    glm::vec3 tan={.80f,.60f,.35f}, tnd={.60f,.42f,.22f};
    glm::vec3 crm={.98f,.95f,.85f}, wht={1,1,1};
    glm::vec3 blk={.10f,.10f,.10f}, pnk={1,.6f,.7f};
    glm::vec3 brn={.45f,.28f,.10f};

    // Body (barrel chest tapering to hips)
    renderer.drawCube({0,.50f,.18f},{.62f,.24f,.35f},tan);
    renderer.drawCube({0,.48f,0},   {.60f,.22f,.40f},tan);
    renderer.drawCube({0,.46f,-.22f},{.52f,.20f,.30f},tan);
    renderer.drawCube({0,.34f,.05f},{.44f,.10f,.52f},crm); // belly
    renderer.drawCube({0,.62f,0},   {.50f,.10f,.55f},tnd); // back ridge

    // Neck
    renderer.drawCube({0,.74f,.28f},{.30f,.18f,.28f},tan);
    renderer.drawCube({0,.82f,.32f},{.26f,.12f,.24f},tan);

    // 4 Legs: front (z=.30) and back (z=-.30), both sides
    for(float fz:{.30f,-.30f})
        for(float s:{-1.f,1.f}){
            renderer.drawCube({s*.24f,.34f,fz},{.14f,.18f,.14f},tan);
            renderer.drawCube({s*.24f,.16f,fz},{.13f,.14f,.13f},tnd);
            renderer.drawCube({s*.24f,.04f,fz+.02f},{.16f,.08f,.20f},crm);
        }

    // Tail (perky curl)
    renderer.drawCube({0,.56f,-.40f},{.12f,.14f,.10f},tan);
    renderer.drawCube({0,.70f,-.44f},{.10f,.14f,.08f},tan);
    renderer.drawCube({0,.82f,-.40f},{.09f,.12f,.08f},tnd);
    renderer.drawCube({0,.88f,-.36f},{.08f,.08f,.08f},crm);

    // Head skull + jaw cheeks + top stripe
    renderer.drawCube({0,.98f,.36f},{.52f,.44f,.44f},tan);
    renderer.drawCube({0,.90f,.40f}, {.56f,.30f,.42f},crm);
    renderer.drawCube({0,1.08f,.34f},{.38f,.12f,.36f},tnd);

    // Floppy ears + inner pink (symmetric, 3-seg droop)
    for(float s:{-1.f,1.f}){
        float ex=s*.30f;
        renderer.drawCube({ex,        1.02f,.28f},{.14f,.14f,.12f},tnd);
        renderer.drawCube({ex+s*.02f, .86f,.26f}, {.14f,.16f,.10f},tnd);
        renderer.drawCube({ex+s*.03f, .72f,.28f}, {.12f,.14f,.10f},tnd);
        renderer.drawCube({ex+s*.01f, .86f,.30f}, {.06f,.20f,.05f},pnk);
    }

    // Muzzle
    renderer.drawCube({0,.88f,.62f},{.32f,.24f,.20f},crm);

    // Eyes (brown iris, pupil, shine) + brows (symmetric)
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.17f,1.02f,.60f},{.10f,.10f,.05f},brn);
        renderer.drawCube({s*.17f,1.02f,.63f},{.06f,.06f,.03f},blk);
        renderer.drawCube({s*.17f+.04f,1.06f,.64f},{.03f,.03f,.02f},wht);
        renderer.drawCube({s*.19f,1.12f,.59f},{.07f,.03f,.03f},tnd); // brow
    }

    // Nose + nostrils
    renderer.drawCube({0,.96f,.74f},{.14f,.09f,.07f},blk);
    for(float s:{-1.f,1.f})
        renderer.drawCube({s*.05f,.97f,.77f},{.03f,.03f,.02f},{.30f,.30f,.30f});
}

void CharacterDog::drawExtra(Renderer& renderer, const Chicken& player) const {
    glm::vec3 pnk={1,.6f,.7f}, dpnk={.95f,.45f,.58f};
    // Tongue hangs downward; on death it droops further
    float yOff=.74f, ySz=.14f;
    if(player.getIsDead()){
        float e = std::min(player.getDeathTimer()*4.f, 1.f) * .80f;
        ySz += e;  yOff -= e*.5f;
    }
    renderer.drawCube({0,yOff,.68f},{.13f,ySz,.12f},pnk);
    renderer.drawCube({0,yOff,.71f},{.02f,ySz,.02f},dpnk); // center crease
}