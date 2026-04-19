cmake_minimum_required(VERSION 3.10)
project(CS302N_Game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message(STATUS "Building CS302N Graphics Project")

# Find OpenGL
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found: ${OPENGL_LIBRARIES}")

# Find GLUT
find_package(GLUT REQUIRED)
message(STATUS "GLUT found: ${GLUT_LIBRARIES}")

# Find GLEW
find_package(GLEW REQUIRED)
message(STATUS "GLEW found")

# GLM is header-only
if(EXISTS /opt/homebrew/include/glm)
    set(GLM_INCLUDE_DIR /opt/homebrew/include)
    message(STATUS "GLM found at /opt/homebrew/include")
elseif(EXISTS /usr/local/include/glm)
    set(GLM_INCLUDE_DIR /usr/local/include)
    message(STATUS "GLM found at /usr/local/include")
elseif(EXISTS /usr/include/glm)
    set(GLM_INCLUDE_DIR /usr/include)
    message(STATUS "GLM found at /usr/include")
else()
    message(FATAL_ERROR "GLM not found!")
endif()

# Include directories
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files (UPDATED to include the whole tree)
set(SOURCES
    main.cpp
    src/game.cpp
    src/character.cpp
    src/lane.cpp
    src/obstacle.cpp
    src/collision.cpp
    src/renderer.cpp
    src/camera.cpp
    src/shader.cpp
    src/coin.cpp
    src/stb_implement.cpp
    src/character_chicken.cpp   
    src/character_frog.cpp
    src/character_dino.cpp
    src/character_cat.cpp
    src/character_dog.cpp
    src/character_base.cpp
)

# Create executable
add_executable(CS302N_Game ${SOURCES})

# --- NEW: Define the ASSET_DIR macro for absolute paths ---
target_compile_definitions(CS302N_Game PRIVATE 
    ASSET_DIR="${PROJECT_SOURCE_DIR}/assets/"
)
# ----------------------------------------------------------

# Link libraries
target_link_libraries(CS302N_Game
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    GLEW::GLEW
)

# macOS specific frameworks
if(APPLE)
    target_link_libraries(CS302N_Game "-framework Cocoa")
    target_link_libraries(CS302N_Game "-framework OpenGL")
    target_link_libraries(CS302N_Game "-framework IOKit")
    message(STATUS "macOS frameworks linked")
endif()

message(STATUS "CS302N_Game build configured successfully!")