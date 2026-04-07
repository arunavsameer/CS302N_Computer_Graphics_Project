#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <string>
#include <map>

#include "shader.h"   // ✅ FIX 1: include full shader

class Renderer {
public:
    Renderer();
    ~Renderer();

    void initialize();
    void prepareFrame();

    // ✅ MATCH EXACT SIGNATURES FROM .cpp
    void loadTexture(const char* path, const std::string& name);

    void drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color);

    void drawTexturedCube(glm::vec3 position, glm::vec3 scale, const std::string& textureName, float rotationY = 0.0f);

    // ✅ NEW (sprite with rotation support)
    void drawSprite(glm::vec3 position, glm::vec3 scale, const std::string& textureName, float rotationY = 0.0f);

private:
    std::map<std::string, unsigned int> textures;
    Shader* mainShader;
};

#endif