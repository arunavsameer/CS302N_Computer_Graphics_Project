#include "../include/character_chicken.h"

std::vector<glm::vec3> CharacterChicken::getWaterDeathPalette() const {
    return {{1,1,1},{1,1,1},{1,1,1},{1,.5f,.05f},{1,.5f,.05f},
            {1,.20f,.55f},{.85f,.10f,.10f},{.05f,.05f,.05f}};
}

void CharacterChicken::drawModel(Renderer& renderer) const {
    glm::vec3 wht={1,1,1},    owt={.92f,.92f,.88f};
    glm::vec3 org={1,.50f,.05f}, ord={.85f,.40f,.02f};
    glm::vec3 red={.85f,.10f,.10f}, pnk={1,.20f,.55f};
    glm::vec3 blk={.05f,.05f,.05f}, yel={.95f,.85f,.10f};

    // Body (layered for plump egg shape)
    renderer.drawCube({0,.50f,0},  {.68f,.28f,.68f},wht);
    renderer.drawCube({0,.64f,0},  {.60f,.16f,.60f},wht);
    renderer.drawCube({0,.34f,.02f},{.62f,.14f,.62f},owt); // belly
    renderer.drawCube({0,.50f,.28f},{.44f,.24f,.22f},wht); // breast bulge

    // Wings: 3-layer stubs, symmetric
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.46f,.56f,.04f},{.14f,.24f,.46f},owt);
        renderer.drawCube({s*.52f,.50f,0},   {.08f,.18f,.36f},wht);
        renderer.drawCube({s*.54f,.44f,-.04f},{.06f,.12f,.28f},owt);
    }

    // Tail feathers: base + centre plume + 2 side plumes
    renderer.drawCube({0,.64f,-.38f},{.30f,.14f,.12f},wht);
    renderer.drawCube({0,.78f,-.42f},{.08f,.28f,.08f},wht);
    for(float s:{-1.f,1.f})
        renderer.drawCube({s*.14f,.74f,-.40f},{.08f,.24f,.07f},owt);

    // Neck
    renderer.drawCube({0,.80f,.20f},{.26f,.18f,.24f},wht);
    renderer.drawCube({0,.90f,.22f},{.22f,.12f,.20f},wht);

    // Head + puffed cheeks
    renderer.drawCube({0,1.02f,.28f},{.44f,.40f,.42f},wht);
    for(float s:{-1.f,1.f})
        renderer.drawCube({s*.24f,.98f,.34f},{.10f,.14f,.18f},owt);

    // Comb: base strip + centre lobe (tallest) + 2 side lobes
    renderer.drawCube({0,1.18f,.22f},{.22f,.06f,.10f},red);
    renderer.drawCube({0,1.24f,.24f},{.08f,.12f,.10f},red);
    for(float s:{-1.f,1.f})
        renderer.drawCube({s*.10f,1.20f,.22f},{.07f,.08f,.08f},red);

    // Wattle
    renderer.drawCube({0,.88f,.50f},{.13f,.16f,.08f},pnk);
    renderer.drawCube({0,.80f,.50f},{.10f,.10f,.07f},pnk);

    // Beak (upper + lower)
    renderer.drawCube({0,1.00f,.54f},{.14f,.07f,.14f},org);
    renderer.drawCube({0,.94f,.52f}, {.12f,.06f,.12f},ord);

    // Eyes: yellow iris, pupil, shine – symmetric
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.20f,1.06f,.46f},{.10f,.10f,.06f},yel);
        renderer.drawCube({s*.20f,1.06f,.50f},{.06f,.06f,.04f},blk);
        renderer.drawCube({s*.20f+.04f,1.10f,.52f},{.03f,.03f,.02f},wht);
    }

    // Legs: upper, shin, foot + 3 forward toes + back spur – symmetric
    // Toe offsets: {sideways_offset, z, z_size}
    static const struct{float t,z,sz;}toe[]={{-.08f,.22f,.12f},{0.f,.26f,.14f},{.08f,.22f,.12f}};
    for(float s:{-1.f,1.f}){
        renderer.drawCube({s*.20f,.28f,.04f},{.12f,.18f,.12f},org);
        renderer.drawCube({s*.20f,.12f,.04f},{.10f,.14f,.10f},ord);
        renderer.drawCube({s*.20f,.04f,.10f},{.10f,.06f,.18f},org);
        for(auto& tk:toe)
            renderer.drawCube({s*(.20f+tk.t),.03f,tk.z},{.05f,.04f,tk.sz},ord);
        renderer.drawCube({s*.20f,.03f,-.06f},{.05f,.04f,.10f},ord); // back spur
    }
}