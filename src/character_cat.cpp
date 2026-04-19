#include "../include/character_cat.h"
#include "../include/character.h"
#include <algorithm>

std::vector<glm::vec3> CharacterCat::getWaterDeathPalette() const {
    return {{1,1,1},{1,1,1},{.9f,.5f,.1f},{.15f,.15f,.15f},{1,.6f,.7f},{.05f,.05f,.05f}};
}

void CharacterCat::drawModel(Renderer& renderer) const {
    glm::vec3 wht={1,1,1},  org={.9f,.5f,.1f},  blkp={.15f,.15f,.15f};
    glm::vec3 pnk={1,.6f,.7f}, blk={.05f,.05f,.05f};
    glm::vec3 grn={.2f,.7f,.25f}, crm={.98f,.96f,.88f};

    // Body (asymmetric calico)
    renderer.drawCube({0,.38f,0},    {.55f,.22f,.50f},crm);
    renderer.drawCube({0,.42f,0},    {.60f,.18f,.52f},wht);
    renderer.drawCube({0,.58f,-.05f},{.50f,.10f,.42f},wht);
    renderer.drawCube({.14f,.44f,-.22f},{.30f,.20f,.28f},org);
    renderer.drawCube({-.18f,.48f,.22f},{.28f,.18f,.22f},blkp);
    renderer.drawCube({.08f,.40f,.26f},{.14f,.12f,.08f},org);

    // Neck
    renderer.drawCube({0,.68f,.20f},{.28f,.16f,.25f},wht);
    renderer.drawCube({0,.76f,.22f},{.24f,.10f,.22f},wht);

    // Legs: FL=wht, FR=org, BL=blkp, BR=wht
    const struct{float x,z;glm::vec3 c;} legs[]={
        {-.22f,.26f,wht},{.22f,.26f,org},{-.22f,-.24f,blkp},{.22f,-.24f,wht}};
    for(auto& l:legs){
        renderer.drawCube({l.x,.24f,l.z},{.12f,.18f,.12f},l.c);
        renderer.drawCube({l.x,.06f,l.z},{.13f,.12f,.15f},l.c);
    }

    // Tail (5-segment S-curve)
    renderer.drawCube({0,.46f,-.42f},  {.10f,.10f,.16f},wht);
    renderer.drawCube({.04f,.54f,-.54f},{.09f,.12f,.09f},org);
    renderer.drawCube({.06f,.66f,-.60f},{.09f,.14f,.08f},wht);
    renderer.drawCube({.04f,.78f,-.56f},{.08f,.12f,.08f},org);
    renderer.drawCube({0,.86f,-.50f},  {.07f,.10f,.07f},blkp);

    // Head skull + forehead patch
    renderer.drawCube({0,.94f,.30f},   {.46f,.42f,.40f},wht);
    renderer.drawCube({.12f,1.00f,.32f},{.28f,.28f,.28f},org);

    // Cheeks, ears, whisker puffs (symmetric)
    for(float s:{-1.f,1.f}){
        glm::vec3 ec = s<0?blkp:org;
        renderer.drawCube({s*.24f,.86f,.36f},{.10f,.14f,.18f},s<0?wht:org);  // cheek
        renderer.drawCube({s*.20f,1.14f,.24f},{.12f,.08f,.07f},ec);           // ear base
        renderer.drawCube({s*.20f,1.24f,.22f},{.08f,.08f,.06f},ec);           // ear tip
        renderer.drawCube({s*.20f,1.16f,.27f},{.06f,.08f,.04f},pnk);          // ear inner
        renderer.drawCube({s*.30f,.86f,.50f},{.14f,.06f,.08f},wht);           // whisker puff
    }

    // Muzzle
    renderer.drawCube({0,.84f,.52f},{.26f,.18f,.12f},wht);

    // Eyes (green iris, slit pupil, shine) + mouth corners
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.17f,.94f,.52f},{.10f,.11f,.05f},grn);
        renderer.drawCube({s*.17f,.94f,.55f},{.04f,.10f,.03f},blk);
        renderer.drawCube({s*.17f+.03f,.98f,.56f},{.03f,.03f,.02f},wht);
        renderer.drawCube({s*.05f,.83f,.62f},{.03f,.02f,.03f},blk); // mouth corner
    }

    // Nose
    renderer.drawCube({0,.88f,.60f},{.07f,.05f,.04f},pnk);
}

void CharacterCat::drawExtra(Renderer& renderer, const Chicken& player) const {
    glm::vec3 pnk = {1,.6f,.7f};
    float zOff=.65f, zSz=.08f;
    if(player.getIsDead()){
        float e = std::min(player.getDeathTimer()*4.f, 1.f) * .70f;
        zSz += e;  zOff += e*.5f;
    }
    renderer.drawCube({0,.78f,zOff},{.09f,.04f,zSz},pnk);
}