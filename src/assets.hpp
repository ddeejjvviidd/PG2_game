// #pragma once

// #include <GL/glew.h>
// #ifdef _WIN32
//     #include <GL/wglew.h>
// #endif
// #include <glm/glm.hpp>
// #include <glm/gtc/type_ptr.hpp>

// // vertex description
// struct vertex
// {
//     glm::vec3 position;
// };

#pragma once

#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};
