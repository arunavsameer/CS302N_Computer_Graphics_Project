#ifndef CHARACTER_DOG_H
#define CHARACTER_DOG_H

#include "character_base.h"

class CharacterDog : public CharacterBase {
public:
    std::vector<glm::vec3> getWaterDeathPalette() const override;
protected:
    void drawModel(Renderer& renderer) const override;
    void drawExtra(Renderer& renderer, const Chicken& player) const override;
};

#endif