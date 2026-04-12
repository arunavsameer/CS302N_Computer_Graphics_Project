#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <string>
#include <map>

#include "shader.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void initialize();
    void prepareFrame();

    void loadTexture(const char* path, const std::string& name);

    void drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color);

    void drawTexturedCube(glm::vec3 position, glm::vec3 scale,
                          const std::string& textureName, float rotationY = 0.0f);

    void drawSprite(glm::vec3 position, glm::vec3 scale,
                    const std::string& textureName, float rotationY = 0.0f);

    // Procedural animated water – replaces the static river PNG
    void drawAnimatedWater(glm::vec3 position, glm::vec3 scale);

private:
    std::map<std::string, unsigned int> textures;
    Shader* mainShader;
};

#endif