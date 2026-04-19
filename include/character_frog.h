#ifndef CHARACTER_FROG_H
#define CHARACTER_FROG_H

#include "character_base.h"

class CharacterFrog : public CharacterBase {
public:
    std::vector<glm::vec3> getWaterDeathPalette() const override;
protected:
    void drawModel(Renderer& renderer) const override;
};

#endif 