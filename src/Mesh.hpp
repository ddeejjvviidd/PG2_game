#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "assets.hpp"
#include "ShaderProgram.hpp"

class Mesh
{
public:
    // mesh data
    glm::vec3 origin{};
    glm::vec3 orientation{};

    GLuint texture_id{0}; // texture id=0  means no texture
    GLenum primitive_type = GL_POINT;
    ShaderProgram shader;

    // mesh material
    glm::vec4 ambient_material{1.0f};  // white, non-transparent
    glm::vec4 diffuse_material{1.0f};  // white, non-transparent
    glm::vec4 specular_material{1.0f}; // white, non-transparent
    float reflectivity{1.0f};

    // Default constructor
    Mesh()
        : primitive_type(GL_POINT),
          shader(), // Default-constructed ShaderProgram (ID = 0)
          texture_id(0),
          VAO(0),
          VBO(0),
          EBO(0),
          origin(0.0f),
          orientation(0.0f),
          ambient_material(1.0f),
          diffuse_material(1.0f),
          specular_material(1.0f),
          reflectivity(1.0f)
    {
        // No OpenGL calls here; initialization happens later
    }

    // indirect (indexed) draw
    Mesh(GLenum primitive_type, ShaderProgram shader, std::vector<Vertex> const &vertices, std::vector<GLuint> const &indices, glm::vec3 const &origin, glm::vec3 const &orientation, GLuint const texture_id = 0) : primitive_type(primitive_type),
                                                                                                                                                                                                                     shader(shader),
                                                                                                                                                                                                                     vertices(vertices),
                                                                                                                                                                                                                     indices(indices),
                                                                                                                                                                                                                     origin(origin),
                                                                                                                                                                                                                     orientation(orientation),
                                                                                                                                                                                                                     texture_id(texture_id)
    {
        // TODO: create and initialize VAO, VBO, EBO and parameters
        // Create VAO + data description (just envelope, or container...)
        glCreateVertexArrays(1, &VAO);
        if (VAO == 0)
        {
            std::cerr << "Failed to create VAO\n";
            throw std::runtime_error("VAO creation failed");
        }

        // Activate shader to get attribute locations
        GLuint shader_prog_ID = shader.getID();
        if (shader_prog_ID == 0)
        {
            std::cerr << "Shader program ID is 0\n";
            throw std::runtime_error("Invalid shader program");
        }

        // Get attribute location for position
        GLint position_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Position");
        if (position_attrib_location == -1)
        {
            std::cerr << "Failed to find attribute 'attribute_Position' in shader\n";
            throw std::runtime_error("Invalid attribute location");
        }

        glEnableVertexArrayAttrib(VAO, position_attrib_location);
        glVertexArrayAttribFormat(VAO, position_attrib_location, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Position));
        glVertexArrayAttribBinding(VAO, position_attrib_location, 0); // (GLuint vaobj, GLuint attribindex, GLuint bindingindex)

        // Create and fill data
        glCreateBuffers(1, &VBO);
        if (VBO == 0)
        {
            std::cerr << "Failed to create VBO\n";
            glDeleteVertexArrays(1, &VAO);
            throw std::runtime_error("VBO creation failed");
        }
        glNamedBufferData(VBO, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // Create and fill EBO with index data
        glCreateBuffers(1, &EBO);
        if (EBO == 0)
        {
            std::cerr << "Failed to create EBO\n";
            glDeleteBuffers(1, &VBO);
            glDeleteVertexArrays(1, &VAO);
            throw std::runtime_error("EBO creation failed");
        }
        glNamedBufferData(EBO, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // Connect together
        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex)); // (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
        glVertexArrayElementBuffer(VAO, EBO);                      // Bind EBO to VAO
    };

    GLuint getVAO() const { return VAO; }
    GLsizei getIndexCount() const { return static_cast<GLsizei>(indices.size()); } // Getter for index count

    void draw(glm::vec3 const &offset, glm::vec3 const &rotation)
    {
        if (VAO == 0)
        {
            std::cerr << "VAO not initialized!\n";
            return;
        }

        shader.activate();

        // for future use: set uniform variables: position, textures, etc...
        // set texture id etc...
        // if (texture_id > 0) {
        //    ...
        //}

        // TODO: draw mesh: bind vertex array object, draw all elements with selected primitive type
    }

    void clear(void)
    {
        texture_id = 0;
        primitive_type = GL_POINT;
        // TODO: clear rest of the member variables to safe default

        // TODO: delete all allocations
        // glDeleteBuffers...
        // glDeleteVertexArrays...
    };

private:
    // OpenGL buffer IDs
    // ID = 0 is reserved (i.e. uninitalized)
    unsigned int VAO{0}, VBO{0}, EBO{0};
    // Mesh data stored for reference
    std::vector<Vertex> vertices; // Added member
    std::vector<GLuint> indices;  // Added member
};
