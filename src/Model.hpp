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

    // OBJ constructor (for cubes)
    Model(const std::filesystem::path &filename, ShaderProgram shader, std::string texturePath = "")
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
        meshes.emplace_back(GL_TRIANGLES, shader, texturePath, vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
        name = filename.stem().string(); // Set model name to filename without extension
    }

    // Flat plane constructor (for labyrinth floor)
    Model(float width, float depth, ShaderProgram shader, const std::string &texturePath)
        : shader(shader), name("floor")
    {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        // Create a flat plane (2 triangles forming a quad)
        Vertex v0, v1, v2, v3;
        v0.Position = glm::vec3(-width / 2.0f, 0.0f, -depth / 2.0f); // Bottom-left
        v1.Position = glm::vec3(width / 2.0f, 0.0f, -depth / 2.0f);  // Bottom-right
        v2.Position = glm::vec3(width / 2.0f, 0.0f, depth / 2.0f);   // Top-right
        v3.Position = glm::vec3(-width / 2.0f, 0.0f, depth / 2.0f);  // Top-left

        v0.TexCoords = glm::vec2(0.0f, 0.0f);
        v1.TexCoords = glm::vec2(1.0f, 0.0f);
        v2.TexCoords = glm::vec2(1.0f, 1.0f);
        v3.TexCoords = glm::vec2(0.0f, 1.0f);

        v0.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        v1.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        v2.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        v3.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);

        // Two triangles: 0-1-2 and 0-2-3
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(3);

        meshes.emplace_back(GL_TRIANGLES, shader, texturePath, vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
    }

    // Heightmap constructor
    Model(const std::string &heightmapPath, ShaderProgram shader, const std::string &texturePath, int width, int depth, float heightScale)
        : shader(shader), name("heightmap")
    {
        cv::Mat heightmap = cv::imread(heightmapPath, cv::IMREAD_GRAYSCALE);
        if (heightmap.empty())
        {
            std::cerr << "Failed to load heightmap: " << heightmapPath << "\n";
            throw std::runtime_error("Heightmap loading failed");
        }

        if (heightmap.cols != width || heightmap.rows != depth)
        {
            cv::resize(heightmap, heightmap, cv::Size(width, depth));
        }

        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        // Generate vertices
        for (int z = 0; z < depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                Vertex v;
                float height = heightmap.at<uchar>(z, x) / 255.0f * heightScale;
                v.Position = glm::vec3(x - width / 2.0f, height, z - depth / 2.0f);
                v.TexCoords = glm::vec2(static_cast<float>(x) / (width - 1), static_cast<float>(z) / (depth - 1));
                v.Normal = glm::vec3(0.0f, 1.0f, 0.0f); // Placeholder
                vertices.push_back(v);
            }
        }

        // Generate indices for triangles
        for (int z = 0; z < depth - 1; ++z)
        {
            for (int x = 0; x < width - 1; ++x)
            {
                int topLeft = z * width + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * width + x;
                int bottomRight = bottomLeft + 1;

                // First triangle (topLeft -> bottomLeft -> topRight)
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // Second triangle (topRight -> bottomLeft -> bottomRight)
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        meshes.emplace_back(GL_TRIANGLES, shader, texturePath, vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
    }

    // update position etc. based on running time
    void update(const float totalTime)
    {
        if (name == "cube")
        {
            // Rotate around all axes
            // orientation.x = fmod(totalTime * 30.0f, 360.0f); // X-axis: 30°/sec
            // orientation.y = fmod(totalTime * 45.0f, 360.0f); // Y-axis: 45°/sec (same as triangles)
            // orientation.z = fmod(totalTime * 60.0f, 360.0f); // Z-axis: 60°/sec
        }
        else if (name != "heightmap") // No rotation for heightmap
        {
            // orientation.y = fmod(totalTime * 45.0f, 360.0f);
        }
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
