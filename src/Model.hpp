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
    enum Type
    {
        OBJECT,
        FLAT_FLOOR,
        HEIGHTMAP
    };
    Type type = OBJECT;

    std::vector<Mesh> meshes;
    std::string name;
    glm::vec3 origin{};
    glm::vec3 orientation{};
    ShaderProgram shader;

    float width = 0.0f;
    float depth = 0.0f;
    float heightScale = 1.0f;
    std::vector<float> heightData;

    bool transparent = false; // Flag to indicate if the model is transparent

    bool isSun = false;

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
        : shader(shader), name("floor"), type(FLAT_FLOOR), width(width), depth(depth)
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

        v0.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
        v1.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
        v2.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
        v3.Normal = glm::vec3(1.0f, 0.0f, 0.0f);

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
        : shader(shader), name("heightmap"), type(HEIGHTMAP), 
        width(static_cast<float>(width - 1)),
        depth(static_cast<float>(depth - 1)),
        heightScale(heightScale)
    {
        cv::Mat heightmap = cv::imread(heightmapPath, cv::IMREAD_GRAYSCALE);
        if (heightmap.empty())
        {
            std::cerr << "Failed to load heightmap: " << heightmapPath << "\n";
            throw std::runtime_error("Heightmap loading failed");
        }

        // Resize heightmap to match the specified width and depth
        if (heightmap.cols != width || heightmap.rows != depth)
        {
            cv::resize(heightmap, heightmap, cv::Size(width, depth));
        }

        // store height data normalized [0,1]
        heightData.resize(width * depth);
        for (int z = 0; z < depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                heightData[z * width + x] = heightmap.at<uchar>(z, x) / 255.0f;
            }
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
                v.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
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

    // Sphere constructor
    Model(int segments, ShaderProgram shader, glm::vec3 color)
        : shader(shader), name("sphere")
    {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        // Sphere generation algorithm
        const float PI = 3.1415926f;
        for (int i = 0; i <= segments; ++i)
        {
            float vAngle = PI * i / segments;
            for (int j = 0; j <= segments; ++j)
            {
                float hAngle = 2 * PI * j / segments;

                Vertex vert;
                vert.Position = glm::vec3(
                    sin(vAngle) * cos(hAngle),
                    cos(vAngle),
                    sin(vAngle) * sin(hAngle));
                vert.Normal = vert.Position; // Normals equal to positions for sphere
                vert.TexCoords = glm::vec2(j / (float)segments, i / (float)segments);

                vertices.push_back(vert);
            }
        }

        // Generate indices
        for (int i = 0; i < segments; ++i)
        {
            for (int j = 0; j < segments; ++j)
            {
                int first = i * (segments + 1) + j;
                int second = first + segments + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        meshes.emplace_back(GL_TRIANGLES, shader, "NONE", vertices, indices, // Special "NONE" keyword
                            glm::vec3(0.0f), glm::vec3(0.0f));
        meshes.back().diffuse_material = glm::vec4(color, 1.0f);
        meshes.back().ambient_material = glm::vec4(color, 1.0f);
        meshes.back().specular_material = glm::vec4(1.0f);
    }

    // Height sampling function
    float getHeightAt(float worldX, float worldZ) const
    {   
        if (type != HEIGHTMAP)
            return 0.0f;

        // Convert world coordinates to local coordinates
        float localX = worldX - origin.x;
        float localZ = worldZ - origin.z;

        // Convert to UV coordinates
        float u = (localX / width) + 0.5f;
        float v = (localZ / depth) + 0.5f;

        // Clamp to valid range
        u = glm::clamp(u, 0.0f, 1.0f);
        v = glm::clamp(v, 0.0f, 1.0f);

        // Calculate exact position in height data
        float xPos = u * (width);
        float zPos = v * (depth);

        // Bilinear interpolation
        int x0 = static_cast<int>(xPos);
        int x1 = x0 + 1;
        int z0 = static_cast<int>(zPos);
        int z1 = z0 + 1;

        // Clamp indices
        x0 = glm::clamp(x0, 0, static_cast<int>(width) - 1);
        x1 = glm::clamp(x1, 0, static_cast<int>(width) - 1);
        z0 = glm::clamp(z0, 0, static_cast<int>(depth) - 1);
        z1 = glm::clamp(z1, 0, static_cast<int>(depth) - 1);

        // Get heights
        float h00 = heightData[z0 * width + x0];
        float h01 = heightData[z1 * width + x0];
        float h10 = heightData[z0 * width + x1];
        float h11 = heightData[z1 * width + x1];

        // Interpolation factors
        float xFactor = xPos - x0;
        float zFactor = zPos - z0;

        // Interpolate
        float top = h00 * (1 - xFactor) + h10 * xFactor;
        float bottom = h01 * (1 - xFactor) + h11 * xFactor;
        return (top * (1 - zFactor) + bottom * zFactor) * heightScale;
    }

    // Add normal calculation for heightmap
    glm::vec3 getNormalAt(float worldX, float worldZ) const
    {
        if (type != HEIGHTMAP)
            return glm::vec3(0.0f, 1.0f, 0.0f);

        const float epsilon = 0.1f;
        float height = getHeightAt(worldX, worldZ);
        float dx = getHeightAt(worldX + epsilon, worldZ) - height;
        float dz = getHeightAt(worldX, worldZ + epsilon) - height;

        glm::vec3 tangent(1.0f, dx / epsilon, 0.0f);
        glm::vec3 bitangent(0.0f, dz / epsilon, 1.0f);
        return glm::normalize(glm::cross(tangent, bitangent));
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
        for (auto const &mesh : meshes)
        {
            mesh.draw(origin + offset, orientation + rotation, isSun);
        }
    }
};
