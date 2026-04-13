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

    void drawSprite(glm::vec3 position, glm::vec3 scale, const std::string& textureName, float rotationY = 0.0f);

    void drawAnimatedWater(glm::vec3 position, glm::vec3 scale);
    
    void drawLilypad(glm::vec3 position, glm::vec3 size, glm::vec3 centerColor, glm::vec3 edgeColor);

    void drawEgg(int clicks);

    void drawSignalPost(glm::vec3 base, bool lightRed, bool lightGreen);

private:
    std::map<std::string, unsigned int> textures;
    Shader* mainShader;
};

#endif