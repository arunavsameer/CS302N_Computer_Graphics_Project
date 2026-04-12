#include "../include/renderer.h"
#include "../include/stb_image.h"
#include <GL/glew.h>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#include <iostream>

#ifndef ASSET_DIR
#define ASSET_DIR "../../assets/" 
#endif

Renderer::Renderer() {
    mainShader = nullptr;
}

Renderer::~Renderer() {
    if (mainShader) {
        delete mainShader;
        mainShader = nullptr;
    }
}

void Renderer::initialize() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    std::string vertShader = std::string(ASSET_DIR) + "shaders/vertex.glsl";
    std::string fragShader = std::string(ASSET_DIR) + "shaders/fragment.glsl";
    mainShader = new Shader(vertShader.c_str(), fragShader.c_str());
    
    std::string grassTex = std::string(ASSET_DIR) + "textures/grass.png";
    std::string grass2Tex = std::string(ASSET_DIR) + "textures/grass2.png";
    std::string roadTex = std::string(ASSET_DIR) + "textures/road.png";
    std::string chickenTex = std::string(ASSET_DIR) + "textures/chicken.png";
    std::string carTex = std::string(ASSET_DIR) + "textures/car.png";
    std::string railTex = std::string(ASSET_DIR) + "textures/rail.png";
    std::string riverTex = std::string(ASSET_DIR) + "textures/river.png";
    std::string trainTex = std::string(ASSET_DIR) + "textures/train.png";
    std::string logTex = std::string(ASSET_DIR) + "textures/log.png";

    loadTexture(grassTex.c_str(), "grass");
    loadTexture(grass2Tex.c_str(), "grass2");
    loadTexture(roadTex.c_str(), "road");
    loadTexture(chickenTex.c_str(), "chicken");
    loadTexture(carTex.c_str(), "car");
    loadTexture(railTex.c_str(), "rail");
    loadTexture(riverTex.c_str(), "river");
    loadTexture(trainTex.c_str(), "train");
    loadTexture(logTex.c_str(), "log");

    glClearColor(0.29f, 0.59f, 0.86f, 1.0f);
}

void Renderer::loadTexture(const char* path, const std::string& name) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 4); 
    
    if (data) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        textures[name] = textureID;
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
}

void Renderer::prepareFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawCube(glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef(scale.x, scale.y, scale.z);
    glColor3f(color.r, color.g, color.b);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void Renderer::drawSprite(glm::vec3 position, glm::vec3 scale, const std::string& textureName, float rotationY) {
    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationY, 0, 1, 0);
    glScalef(scale.x, scale.y, scale.z);

    if (mainShader) {
        mainShader->use();
        mainShader->setInt("texture1", 0);
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[textureName]);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.05f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.05f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.05f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.05f);
    glEnd();

    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.05f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.05f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.05f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.05f);
    glEnd();

    glDisable(GL_BLEND);

    if (mainShader) glUseProgram(0);

    glPopMatrix();
}

void Renderer::drawTexturedCube(glm::vec3 position, glm::vec3 scale, const std::string& textureName, float rotationY) {
    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);

    if (rotationY != 0.0f)
        glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

    glScalef(scale.x, scale.y, scale.z);

    // 1. Set repetition multipliers based on the object's scale
    // This makes 1 cell exactly 1 repeating texture tile.
    float rx = scale.x;
    float ry = scale.y;
    float rz = scale.z;

    if (textureName == "log") {
        rx *= 2.0f; // Preserving your existing log specific logic
    }

    if (mainShader) {
        mainShader->use();
        mainShader->setInt("texture1", 0);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[textureName]);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

        // FRONT
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
        glTexCoord2f(rx,   0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
        glTexCoord2f(rx,   ry);   glVertex3f( 0.5f,  0.5f,  0.5f);
        glTexCoord2f(0.0f, ry);   glVertex3f(-0.5f,  0.5f,  0.5f);

        // BACK
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glTexCoord2f(rx,   0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
        glTexCoord2f(rx,   ry);   glVertex3f( 0.5f,  0.5f, -0.5f);
        glTexCoord2f(0.0f, ry);   glVertex3f(-0.5f,  0.5f, -0.5f);

        // TOP (This is the ground we walk on)
        if (textureName == "rail") {
            // Your rail.png has vertical metal rails, but the lane spans horizontally.
            // We rotate the UV map 90 degrees here so the tracks run left-to-right!
            glTexCoord2f(rz,   rx);   glVertex3f(-0.5f,  0.5f, -0.5f);
            glTexCoord2f(0.0f, rx);   glVertex3f(-0.5f,  0.5f,  0.5f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
            glTexCoord2f(rz,   0.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
        } else {
            // Standard tiling for Grass, Road, and River
            glTexCoord2f(0.0f, rz);   glVertex3f(-0.5f,  0.5f, -0.5f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
            glTexCoord2f(rx,   0.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
            glTexCoord2f(rx,   rz);   glVertex3f( 0.5f,  0.5f, -0.5f);
        }

        // BOTTOM
        glTexCoord2f(rx,   rz);   glVertex3f(-0.5f, -0.5f, -0.5f);
        glTexCoord2f(0.0f, rz);   glVertex3f( 0.5f, -0.5f, -0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
        glTexCoord2f(rx,   0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);

        // RIGHT
        glTexCoord2f(rz,   0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
        glTexCoord2f(rz,   ry);   glVertex3f( 0.5f,  0.5f, -0.5f);
        glTexCoord2f(0.0f, ry);   glVertex3f( 0.5f,  0.5f,  0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);

        // LEFT
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glTexCoord2f(rz,   0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
        glTexCoord2f(rz,   ry);   glVertex3f(-0.5f,  0.5f,  0.5f);
        glTexCoord2f(0.0f, ry);   glVertex3f(-0.5f,  0.5f, -0.5f);

    glEnd();

    if (mainShader) glUseProgram(0);

    glPopMatrix();
}