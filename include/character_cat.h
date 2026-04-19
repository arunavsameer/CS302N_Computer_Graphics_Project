#ifndef CHARACTER_CAT_H
#define CHARACTER_CAT_H

#include "character_base.h"

class CharacterCat : public CharacterBase {
public:
    std::vector<glm::vec3> getWaterDeathPalette() const override;
protected:
    void drawModel(Renderer& renderer) const override;
    void drawExtra(Renderer& renderer, const Chicken& player) const override;
};

#endif