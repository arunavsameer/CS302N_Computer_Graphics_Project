#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include "shader.h"

class Renderer {
private:
    Shader* mainShader;
    std::unordered_map<std::string, unsigned int> textures;
    
    void loadTexture(const char* path, const std::string& name);

public:
    void initialize();
    void prepareFrame();
    
    // Core drawing functions
    void drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color);
    
    // ADDED: rotationY with a default of 0.0f
    void drawTexturedCube(glm::vec3 position, glm::vec3 scale, const std::string& textureName, float rotationY = 0.0f);
};

#endif