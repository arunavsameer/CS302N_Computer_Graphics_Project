#ifndef SHADER_H
#define SHADER_H

#include <string>

class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    void setInt(const std::string &name, int value) const;
};

#endif