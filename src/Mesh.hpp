#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "assets.hpp"
#include "ShaderProgram.hpp"
#include <opencv2/opencv.hpp>

// Represents a 3D mesh with vertex data, material properties, and OpenGL resources.
class Mesh
{
public:
    // Mesh transformation properties.
    glm::vec3 origin{};      // Position of the mesh's origin in world space.
    glm::vec3 orientation{}; // Euler angles (degrees) for mesh rotation.

    // OpenGL rendering properties.
    GLuint texture_id{0};  // ID of the texture; 0 indicates no texture.
    GLenum primitive_type; // OpenGL primitive type (e.g., GL_TRIANGLES, GL_POINTS).
    ShaderProgram shader;  // Shader program for rendering the mesh.

    // Material properties for lighting calculations.
    glm::vec4 ambient_material{1.0f};  // Ambient color and opacity (RGBA, default white).
    glm::vec4 diffuse_material{1.0f};  // Diffuse color and opacity (RGBA, default white).
    glm::vec4 specular_material{1.0f}; // Specular color and opacity (RGBA, default white).
    float reflectivity{1.0f};          // Shininess factor for specular highlights.

    // Default constructor initializing a mesh with safe defaults.
    Mesh()
        : primitive_type(GL_POINTS), // Default to point rendering.
          shader(),                  // Initialize shader with invalid ID (0).
          texture_id(0),             // No texture by default.
          VAO(0),                    // Uninitialized VAO.
          VBO(0),                    // Uninitialized VBO.
          EBO(0),                    // Uninitialized EBO.
          origin(0.0f),              // Origin at (0,0,0).
          orientation(0.0f),         // No rotation.
          ambient_material(1.0f),    // White, opaque ambient material.
          diffuse_material(1.0f),    // White, opaque diffuse material.
          specular_material(1.0f),   // White, opaque specular material.
          reflectivity(1.0f)         // Default shininess.
    {
        // Defer OpenGL resource creation to avoid context issues.
    }

    // Constructor for indexed drawing with vertex and index data.
    Mesh(GLenum primitive_type, ShaderProgram shader, std::string texturePath,
         std::vector<Vertex> const &vertices, std::vector<GLuint> const &indices,
         glm::vec3 const &origin, glm::vec3 const &orientation, GLuint const texture_id = 0)
        : primitive_type(primitive_type),
          shader(shader),
          texture_id(texture_id),
          vertices(vertices),
          indices(indices),
          origin(origin),
          orientation(orientation)
    {
        // Create and bind Vertex Array Object (VAO).
        glCreateVertexArrays(1, &VAO);
        if (VAO == 0)
        {
            std::cerr << "Error: Failed to create VAO\n";
            throw std::runtime_error("VAO creation failed");
        }

        // Validate shader program.
        GLuint shader_prog_ID = shader.getID();
        if (shader_prog_ID == 0)
        {
            std::cerr << "Error: Invalid shader program ID\n";
            throw std::runtime_error("Invalid shader program");
        }

        // Configure position attribute.
        GLint position_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Position");
        if (position_attrib_location == -1)
        {
            std::cerr << "Error: Shader lacks 'attribute_Position'\n";
            throw std::runtime_error("Invalid position attribute");
        }
        glEnableVertexArrayAttrib(VAO, position_attrib_location);
        glVertexArrayAttribFormat(VAO, position_attrib_location, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Position));
        glVertexArrayAttribBinding(VAO, position_attrib_location, 0);

        // Configure normal attribute if used by shader.
        GLint normal_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Normal");
        if (normal_attrib_location != -1)
        {
            glEnableVertexArrayAttrib(VAO, normal_attrib_location);
            glVertexArrayAttribFormat(VAO, normal_attrib_location, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Normal));
            glVertexArrayAttribBinding(VAO, normal_attrib_location, 0);
        }

        // Configure texture coordinate attribute if used by shader.
        GLint texcoord_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_TexCoords");
        if (texcoord_attrib_location != -1)
        {
            glEnableVertexArrayAttrib(VAO, texcoord_attrib_location);
            glVertexArrayAttribFormat(VAO, texcoord_attrib_location, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, TexCoords));
            glVertexArrayAttribBinding(VAO, texcoord_attrib_location, 0);
        }

        // Create and upload Vertex Buffer Object (VBO).
        glCreateBuffers(1, &VBO);
        if (VBO == 0)
        {
            std::cerr << "Error: Failed to create VBO\n";
            glDeleteVertexArrays(1, &VAO);
            throw std::runtime_error("VBO creation failed");
        }
        glNamedBufferData(VBO, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // Create and upload Element Buffer Object (EBO).
        glCreateBuffers(1, &EBO);
        if (EBO == 0)
        {
            std::cerr << "Error: Failed to create EBO\n";
            glDeleteBuffers(1, &VBO);
            glDeleteVertexArrays(1, &VAO);
            throw std::runtime_error("EBO creation failed");
        }
        glNamedBufferData(EBO, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // Link VBO and EBO to VAO.
        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(VAO, EBO);

        // Load texture if a valid path is provided.
        if (!texturePath.empty())
        {
            loadTexture(texturePath);
        }
    }

    // Returns the Vertex Array Object ID.
    GLuint getVAO() const { return VAO; }

    // Returns the number of indices for indexed drawing.
    GLsizei getIndexCount() const { return static_cast<GLsizei>(indices.size()); }

    // Renders the mesh with specified transformations.
    void draw(glm::vec3 const &offset, glm::vec3 const &rotation, bool isSun = false) const
    {
        if (VAO == 0)
        {
            std::cerr << "Error: VAO not initialized\n";
            return;
        }

        // Activate shader program.
        shader.activate();

        // Compute model matrix with translation, rotation, and scaling.
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, origin + offset);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(orientation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(orientation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(orientation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f)); // Uniform scale factor.

        // Upload model matrix to shader.
        GLint modelLoc = glGetUniformLocation(shader.getID(), "uM_m");
        if (modelLoc != -1)
        {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        }
        else
        {
            std::cerr << "Warning: Shader uniform 'uM_m' not found\n";
        }

        // Upload material properties to shader.
        GLint matAmbientLoc = glGetUniformLocation(shader.getID(), "material.ambient");
        if (matAmbientLoc != -1)
            glUniform3fv(matAmbientLoc, 1, glm::value_ptr(glm::vec3(ambient_material)));

        GLint matDiffuseLoc = glGetUniformLocation(shader.getID(), "material.diffuse");
        if (matDiffuseLoc != -1)
            glUniform3fv(matDiffuseLoc, 1, glm::value_ptr(glm::vec3(diffuse_material)));

        GLint matSpecularLoc = glGetUniformLocation(shader.getID(), "material.specular");
        if (matSpecularLoc != -1)
            glUniform3fv(matSpecularLoc, 1, glm::value_ptr(glm::vec3(specular_material)));

        GLint matShininessLoc = glGetUniformLocation(shader.getID(), "material.shininess");
        if (matShininessLoc != -1)
            glUniform1f(matShininessLoc, reflectivity);

        // Bind texture if available.
        if (texture_id != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glUniform1i(glGetUniformLocation(shader.getID(), "textureSampler"), 0);
        }

        // Draw the mesh using indexed rendering.
        glBindVertexArray(VAO);
        glDrawElements(primitive_type, getIndexCount(), GL_UNSIGNED_INT, 0);
    }

    // Releases OpenGL resources and resets member variables.
    void clear()
    {
        // Reset member variables to defaults.
        texture_id = 0;
        primitive_type = GL_POINTS;
        origin = glm::vec3(0.0f);
        orientation = glm::vec3(0.0f);
        ambient_material = glm::vec4(1.0f);
        diffuse_material = glm::vec4(1.0f);
        specular_material = glm::vec4(1.0f);
        reflectivity = 1.0f;
        vertices.clear();
        indices.clear();

        // Clean up OpenGL resources.
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
    }

private:
    // OpenGL resource IDs (0 indicates uninitialized).
    unsigned int VAO{0}; // Vertex Array Object.
    unsigned int VBO{0}; // Vertex Buffer Object.
    unsigned int EBO{0}; // Element Buffer Object.

    // Mesh geometry data.
    std::vector<Vertex> vertices; // Vertex attributes (position, normal, texcoords).
    std::vector<GLuint> indices;  // Indices for indexed drawing.

    // Loads a texture from a file or creates a default texture.
    void loadTexture(const std::string &texturePath)
    {
        if (texturePath == "NONE")
        {
            // Create a 1x1 yellow texture.
            unsigned char yellow[] = {255, 255, 0, 255};
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, yellow);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }

        if (texturePath.empty())
        {
            // Create a 1x1 white texture.
            unsigned char white[] = {255, 255, 255, 255};
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }

        // Load texture image using OpenCV.
        cv::Mat image = cv::imread(texturePath, cv::IMREAD_UNCHANGED);
        if (image.empty())
        {
            std::cerr << "Error: Failed to load texture: " << texturePath << "\n";
            return;
        }

        // Flip image vertically to match OpenGL's bottom-left origin.
        cv::flip(image, image, 0);

        // Create and configure texture.
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload texture data based on channel count.
        if (image.channels() == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.cols, image.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
            std::cout << "Loaded texture with alpha channel\n";
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);
        }

        // Generate mipmaps and unbind texture.
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};
