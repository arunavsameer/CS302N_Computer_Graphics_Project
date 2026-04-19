#include "../include/character_frog.h"
#include "../include/character.h"
#include <algorithm>

std::vector<glm::vec3> CharacterFrog::getWaterDeathPalette() const
{
    return {{.35f, .70f, .30f}, {.35f, .70f, .30f}, {.35f, .70f, .30f}, {.20f, .50f, .15f}, {.20f, .50f, .15f}, {.85f, .90f, .70f}, {1, 1, 1}, {.05f, .05f, .05f}, {.90f, .10f, .10f}};
}

void CharacterFrog::drawModel(Renderer &renderer) const
{
    glm::vec3 grn = {.35f, .70f, .30f}, grd = {.20f, .50f, .15f};
    glm::vec3 grm = {.28f, .60f, .22f}, bly = {.85f, .90f, .70f};
    glm::vec3 wht = {1, 1, 1}, blk = {.05f, .05f, .05f};
    glm::vec3 gld = {.90f, .75f, .10f}, mth = {.18f, .44f, .12f};

    // Body (Dropped lower: belly now sinks slightly into the ground for a squashed look)
    renderer.drawCube({0, .12f, 0}, {.78f, .20f, .72f}, grn);
    renderer.drawCube({0, .22f, 0}, {.66f, .14f, .62f}, grd);    // back
    renderer.drawCube({0, .02f, .05f}, {.64f, .12f, .62f}, bly); // belly
    for (float s : {-1.f, 1.f})
        renderer.drawCube({s * .40f, .12f, .02f}, {.10f, .18f, .42f}, grm); // side bumps

    // Head
    renderer.drawCube({0, .22f, .32f}, {.72f, .22f, .44f}, grn);
    renderer.drawCube({0, .32f, .30f}, {.66f, .12f, .40f}, grd); // top
    renderer.drawCube({0, .10f, .38f}, {.60f, .12f, .36f}, bly); // chin
    renderer.drawCube({0, .15f, .54f}, {.22f, .03f, .04f}, mth); // mouth line
    for (float s : {-1.f, 1.f})
        renderer.drawCube({s * .28f, .16f, .52f}, {.06f, .05f, .05f}, mth); // mouth corners

    // Bulging eyes
    for (float s : {-1.f, 1.f})
    {
        renderer.drawCube({s * .26f, .44f, .28f}, {.22f, .22f, .22f}, wht);        // dome
        renderer.drawCube({s * .26f, .34f, .22f}, {.26f, .10f, .26f}, grd);        // rim
        renderer.drawCube({s * .26f, .44f, .38f}, {.14f, .14f, .08f}, gld);        // iris
        renderer.drawCube({s * .26f, .44f, .42f}, {.06f, .10f, .05f}, blk);        // pupil
        renderer.drawCube({s * .26f + .05f, .50f, .42f}, {.04f, .04f, .03f}, wht); // shine
        renderer.drawCube({s * .40f, .34f, .14f}, {.12f, .12f, .04f}, grm);        // tympanum
    }

    // Front arms (Shoulders dropped, hands stay planted)
    for (float s : {-1.f, 1.f})
    {
        renderer.drawCube({s * .42f, .08f, .30f}, {.16f, .14f, .20f}, grd); // upper
        renderer.drawCube({s * .44f, .05f, .42f}, {.14f, .10f, .16f}, grm); // forearm
        renderer.drawCube({s * .48f, .04f, .52f}, {.18f, .06f, .14f}, grn); // hand
        renderer.drawCube({s * .44f, .03f, .58f}, {.14f, .04f, .08f}, bly); // webbing
    }

    // Back legs (Thighs dropped, feet stay planted)
    for (float s : {-1.f, 1.f})
    {
        renderer.drawCube({s * .50f, .12f, -.16f}, {.20f, .24f, .28f}, grd);  // thigh
        renderer.drawCube({s * .46f, .06f, -.36f}, {.18f, .18f, .22f}, grm);  // shin
        renderer.drawCube({s * .38f, .04f, -.48f}, {.26f, .08f, .30f}, grn);  // foot
        renderer.drawCube({s * .36f, .03f, -.58f}, {.30f, .05f, .14f}, grn);  // toe spread
        renderer.drawCube({s * .36f, .015f, -.56f}, {.24f, .03f, .10f}, bly); // webbing
        for (float t : {-.12f, 0.f, .12f})
            renderer.drawCube({s * .36f + t, .02f, -.64f}, {.05f, .04f, .08f}, grd); // toe tips
    }
}

void CharacterFrog::drawExtra(Renderer &renderer, const Chicken &player) const
{
    glm::vec3 red = {.90f, .10f, .10f};
    
    // Pushed zOff from .45f forward to .58f so the tongue physically protrudes from the mouth
    float zOff = .58f, zSz = .15f; 
    
    if (player.getIsDead())
    {
        float e = std::min(player.getDeathTimer() * 4.f, 1.f) * .80f;
        zSz += e;
        zOff += e * .5f;
    }
    
    // Y-coordinate aligned perfectly with the new mouth line at .15f
    renderer.drawCube({0, .15f, zOff}, {.08f, .03f, zSz}, red);
}