#ifndef CHARACTER_DINO_H
#define CHARACTER_DINO_H

#include "character_base.h"

class CharacterDino : public CharacterBase {
public:
    std::vector<glm::vec3> getWaterDeathPalette() const override;
protected:
    void drawModel(Renderer& renderer) const override;
};

#endif