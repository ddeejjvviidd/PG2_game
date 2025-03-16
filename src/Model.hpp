#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"

class Model
{
public:
    std::vector<Mesh> meshes;
    std::string name;
    glm::vec3 origin{};
    glm::vec3 orientation{};
    ShaderProgram shader;

    Model() = default; // Add default constructor

    Model(const std::filesystem::path &filename, ShaderProgram shader)
        : shader(shader) // Initialize shader with the passed argument
    {
        // load mesh (all meshes) of the model, (in the future: load material of each mesh, load textures...)
        // call LoadOBJFile, LoadMTLFile (if exist), process data, create mesh and set its properties
        //    notice: you can load multiple meshes and place them to proper positions,
        //            multiple textures (with reusing) etc. to construct single complicated Model
        std::vector<glm::vec3> out_vertices;
        std::vector<glm::vec2> out_uvs;
        std::vector<glm::vec3> out_normals;

        // Load the OBJ file using your loader
        if (!loadOBJ(filename.string().c_str(), out_vertices, out_uvs, out_normals))
        {
            std::cerr << "Failed to load OBJ file: " << filename << "\n";
            throw std::runtime_error("OBJ loading failed");
        }

        // Ensure the sizes match (since your loader unrolls indices)
        if (out_vertices.size() != out_uvs.size() || out_vertices.size() != out_normals.size())
        {
            std::cerr << "Mismatch in vertex/UV/normal counts: " << out_vertices.size() << ", " << out_uvs.size() << ", " << out_normals.size() << "\n";
            throw std::runtime_error("Invalid OBJ data");
        }

        // Convert loaded data into Vertex objects
        std::vector<Vertex> vertices;
        for (size_t i = 0; i < out_vertices.size(); ++i)
        {
            Vertex v;
            v.Position = out_vertices[i];
            v.Normal = out_normals[i];
            v.TexCoords = out_uvs[i];
            vertices.push_back(v);
        }

        // Generate indices (since loadOBJ unrolls them, we create a simple 0, 1, 2, ... sequence)
        std::vector<GLuint> indices;
        for (GLuint i = 0; i < static_cast<GLuint>(vertices.size()); ++i)
        {
            indices.push_back(i);
        }

        // Create a Mesh and add it to the meshes vector
        meshes.emplace_back(GL_TRIANGLES, shader, vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
        name = filename.stem().string(); // Set model name to filename without extension
    }

    // update position etc. based on running time
    void update(const float totalTime)
    {
        // origin += glm::vec3(3,0,0) * delta_t; // s = s0 + v*dt
        // Rotate around the Y-axis (in degrees)
        //orientation.y += 45.0f * delta_t; // 45 degrees per second
        // Keep orientation.y in [0, 360) to avoid overflow (optional)
        //if (orientation.y >= 360.0f)
        //{
        //    orientation.y -= 360.0f;
        //}

        orientation.y = fmod(totalTime * 45.0f, 360.0f);
    }

    void draw(glm::vec3 const &offset = glm::vec3(0.0), glm::vec3 const &rotation = glm::vec3(0.0f))
    {
        // call draw() on mesh (all meshes)
        for (auto const &mesh : meshes)
        {
            mesh.draw(origin + offset, orientation + rotation);
        }
    }
};
