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
        // Create VAO
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

        // Position attribute
        GLint position_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Position");
        if (position_attrib_location == -1)
        {
            std::cerr << "Failed to find attribute 'attribute_Position' in shader\n";
            throw std::runtime_error("Invalid attribute location");
        }
        glEnableVertexArrayAttrib(VAO, position_attrib_location);
        glVertexArrayAttribFormat(VAO, position_attrib_location, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Position));
        glVertexArrayAttribBinding(VAO, position_attrib_location, 0);

        // Normal attribute
        GLint normal_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Normal");
        if (normal_attrib_location != -1) // Only set up if the shader uses it
        {
            glEnableVertexArrayAttrib(VAO, normal_attrib_location);
            glVertexArrayAttribFormat(VAO, normal_attrib_location, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Normal));
            glVertexArrayAttribBinding(VAO, normal_attrib_location, 0);
        }

        // Texture coordinate attribute
        GLint texcoord_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_TexCoords");
        if (texcoord_attrib_location != -1) // Only set up if the shader uses it
        {
            glEnableVertexArrayAttrib(VAO, texcoord_attrib_location);
            glVertexArrayAttribFormat(VAO, texcoord_attrib_location, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, TexCoords));
            glVertexArrayAttribBinding(VAO, texcoord_attrib_location, 0);
        }

        // Create and fill VBO
        glCreateBuffers(1, &VBO);
        if (VBO == 0)
        {
            std::cerr << "Failed to create VBO\n";
            glDeleteVertexArrays(1, &VAO);
            throw std::runtime_error("VBO creation failed");
        }
        glNamedBufferData(VBO, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // Create and fill EBO
        glCreateBuffers(1, &EBO);
        if (EBO == 0)
        {
            std::cerr << "Failed to create EBO\n";
            glDeleteBuffers(1, &VBO);
            glDeleteVertexArrays(1, &VAO);
            throw std::runtime_error("EBO creation failed");
        }
        glNamedBufferData(EBO, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // Connect VBO and EBO to VAO
        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(VAO, EBO);
    };

    GLuint getVAO() const { return VAO; }
    GLsizei getIndexCount() const { return static_cast<GLsizei>(indices.size()); } // Getter for index count

    // void draw(glm::vec3 const &offset, glm::vec3 const &rotation) const
    // {
    //     if (VAO == 0)
    //     {
    //         std::cerr << "VAO not initialized!\n";
    //         return;
    //     }

    //     shader.activate();

    //     // for future use: set uniform variables: position, textures, etc...
    //     // set texture id etc...
    //     // if (texture_id > 0) {
    //     //    ...
    //     //}

    //     // Compute the model matrix based on origin, offset, orientation, and rotation
    //     glm::mat4 model = glm::mat4(1.0f);
    //     model = glm::translate(model, origin + offset); // Apply translation

    //     // Apply rotations (assuming rotation is in degrees for simplicity)
    //     model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // X-axis
    //     model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Y-axis
    //     model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Z-axis

    //     // Assuming orientation is also in degrees (apply model's own orientation)
    //     model = glm::rotate(model, glm::radians(orientation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    //     model = glm::rotate(model, glm::radians(orientation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    //     model = glm::rotate(model, glm::radians(orientation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    //     // Set the model matrix uniform in the shader (assuming the shader has a "model" uniform)
    //     GLint modelLoc = glGetUniformLocation(shader.getID(), "model");
    //     if (modelLoc != -1) // Check if the uniform exists
    //     {
    //         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //     }
    //     else
    //     {
    //         std::cerr << "Warning: 'model' uniform not found in shader\n";
    //     }

    //     // Bind the VAO
    //     glBindVertexArray(VAO);

    //     // Draw the mesh using indexed drawing
    //     glDrawElements(primitive_type, getIndexCount(), GL_UNSIGNED_INT, 0);

    //     // Unbind the VAO (optional, but good practice)
    //     glBindVertexArray(0);
    // }

    void draw(glm::vec3 const &offset, glm::vec3 const &rotation) const
    {
        if (VAO == 0)
        {
            std::cerr << "VAO not initialized!\n";
            return;
        }

        shader.activate();

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, origin + offset);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(orientation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(orientation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(orientation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f)); // Adjust scale if needed

        GLint modelLoc = glGetUniformLocation(shader.getID(), "uM_m"); // Changed to uM_m
        if (modelLoc != -1)
        {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        }
        else
        {
            std::cerr << "Warning: 'uM_m' uniform not found in shader\n";
        }

        glBindVertexArray(VAO);
        glDrawElements(primitive_type, getIndexCount(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void clear(void)
    {
        // Reset member variables to safe defaults
        texture_id = 0;
        primitive_type = GL_POINT;
        origin = glm::vec3(0.0f);
        orientation = glm::vec3(0.0f);
        ambient_material = glm::vec4(1.0f);
        diffuse_material = glm::vec4(1.0f);
        specular_material = glm::vec4(1.0f);
        reflectivity = 1.0f;
        vertices.clear(); // Clear vertex and index data
        indices.clear();

        // Delete OpenGL resources if they exist
        if (EBO != 0)
        {
            glDeleteBuffers(1, &EBO);
            EBO = 0;
        }
        if (VBO != 0)
        {
            glDeleteBuffers(1, &VBO);
            VBO = 0;
        }
        if (VAO != 0)
        {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
    };

private:
    // OpenGL buffer IDs
    // ID = 0 is reserved (i.e. uninitalized)
    unsigned int VAO{0}, VBO{0}, EBO{0};
    // Mesh data stored for reference
    std::vector<Vertex> vertices; // Added member
    std::vector<GLuint> indices;  // Added member
};
