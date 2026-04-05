#include "../include/renderer.h"
#include "../include/stb_image.h"
#include <GL/glew.h>
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#include <iostream>

void Renderer::initialize() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // Initialize Shaders
    mainShader = new Shader("../assets/shaders/vertex.glsl", "../assets/shaders/fragment.glsl");
    
    // Load Textures (Ensure these files exist in your assets folder!)
    loadTexture("../../assets/textures/grass.png", "grass");
    loadTexture("../../assets/textures/road.png", "road");
    loadTexture("../../assets/textures/chicken.png", "chicken");
    loadTexture("../../assets/textures/car.png", "car");

    glClearColor(0.4f, 0.7f, 1.0f, 1.0f);
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
    
    // THE FIX: The '4' at the end forces STB to output RGBA (4 channels)
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 4); 
    
    if (data) {
        // Fix byte alignment issues just to be safe
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        // We now explicitly tell OpenGL the data is GL_RGBA
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

void Renderer::drawTexturedCube(glm::vec3 position, glm::vec3 scale, const std::string& textureName) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef(scale.x, scale.y, scale.z);
    
    if (mainShader) {
        mainShader->use();
        mainShader->setInt("texture1", 0);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[textureName]);
    glColor3f(1.0f, 1.0f, 1.0f); // Pure white so texture colors show correctly

    glBegin(GL_QUADS);
        // Front
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
        // Back
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
        // Top
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
        // Bottom
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
        // Right
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
        // Left
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glEnd();

    if (mainShader) glUseProgram(0);
    glPopMatrix();
}