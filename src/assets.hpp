#pragma once

#include <GL/glew.h>
#ifdef _WIN32
    #include <GL/wglew.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// vertex description
struct vertex
{
    glm::vec3 position;
};
