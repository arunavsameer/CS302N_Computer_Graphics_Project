cmake_minimum_required(VERSION 3.10)
project(ArchOpenGL VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
# Note: We removed find_package(GLEW)

# 1. Include the header files
include_directories(include)

# 2. Add executable
add_executable(OpenGLApp main.cpp src/glad.c src/shader.cpp)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

# 3. Link Libraries
target_link_libraries(
    OpenGLApp
    PRIVATE
    OpenGL::GL
    OpenGL::GLU
    ${GLUT_LIBRARIES}
    glfw
    dl
    glm
)