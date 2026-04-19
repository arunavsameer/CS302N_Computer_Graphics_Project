#ifndef CHARACTER_CHICKEN_H
#define CHARACTER_CHICKEN_H

#include "character_base.h"

class CharacterChicken : public CharacterBase {
public:
    std::vector<glm::vec3> getWaterDeathPalette() const override;
protected:
    void drawModel(Renderer& renderer) const override;
};

#endif