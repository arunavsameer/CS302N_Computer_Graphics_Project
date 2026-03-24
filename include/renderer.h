#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>

class Renderer {
public:
    void initialize();
    void prepareFrame();
    
    // Modular draw calls
    void drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color);
    void drawLane(glm::vec3 position, float width, glm::vec3 color);
};

#endif