#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"

// Represents a 3D model composed of one or more meshes with transformation and rendering properties.
class Model
{
public:
    // Defines the type of model for specialized behavior.
    enum Type
    {
        OBJECT,     // General 3D object (e.g., loaded from OBJ file).
        FLAT_FLOOR, // Flat plane (e.g., for floors or simple surfaces).
        HEIGHTMAP   // Terrain generated from a heightmap image.
    };
    Type type = OBJECT; // Model type, defaults to general object.

    std::vector<Mesh> meshes; // Collection of meshes comprising the model.
    std::string name;         // Name of the model (often derived from file or type).
    glm::vec3 origin{};       // Position of the model in world space.
    glm::vec3 orientation{};  // Euler angles (degrees) for model rotation.
    ShaderProgram shader;     // Shader program used for rendering all meshes.

    float width = 0.0f;            // Width of the model (used for flat floors or heightmaps).
    float depth = 0.0f;            // Depth of the model (used for flat floors or heightmaps).
    float heightScale = 1.0f;      // Scaling factor for heightmap heights.
    std::vector<float> heightData; // Normalized [0,1] height values for heightmap.

    bool transparent = false; // Indicates if the model uses transparency.
    bool isSun = false;       // Indicates if the model is a light source (e.g., sun).

    // Default constructor for an empty model.
    Model() = default;

    // Constructs a model from an OBJ file (e.g., for cubes or complex objects).
    Model(const std::filesystem::path &filename, ShaderProgram shader, std::string texturePath = "")
        : shader(shader), name(filename.stem().string())
    {
        // Load vertex, UV, and normal data from OBJ file.
        std::vector<glm::vec3> out_vertices;
        std::vector<glm::vec2> out_uvs;
        std::vector<glm::vec3> out_normals;
        if (!loadOBJ(filename.string().c_str(), out_vertices, out_uvs, out_normals))
        {
            std::cerr << "Error: Failed to load OBJ file: " << filename << "\n";
            throw std::runtime_error("OBJ loading failed");
        }

        // Validate data consistency.
        if (out_vertices.size() != out_uvs.size() || out_vertices.size() != out_normals.size())
        {
            std::cerr << "Error: Mismatch in vertex/UV/normal counts: " << out_vertices.size()
                      << ", " << out_uvs.size() << ", " << out_normals.size() << "\n";
            throw std::runtime_error("Invalid OBJ data");
        }

        // Convert loaded data into Vertex objects.
        std::vector<Vertex> vertices;
        for (size_t i = 0; i < out_vertices.size(); ++i)
        {
            Vertex v;
            v.Position = out_vertices[i];
            v.Normal = out_normals[i];
            v.TexCoords = out_uvs[i];
            vertices.push_back(v);
        }

        // Generate sequential indices (OBJ loader unrolls indices).
        std::vector<GLuint> indices;
        for (GLuint i = 0; i < static_cast<GLuint>(vertices.size()); ++i)
        {
            indices.push_back(i);
        }

        // Create and store a single mesh for the model.
        meshes.emplace_back(GL_TRIANGLES, shader, texturePath, vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
    }

    // Constructs a flat plane model (e.g., for labyrinth floors).
    Model(float width, float depth, ShaderProgram shader, const std::string &texturePath)
        : shader(shader), name("floor"), type(FLAT_FLOOR), width(width), depth(depth)
    {
        // Define vertices for a quad (two triangles).
        std::vector<Vertex> vertices;
        Vertex v0, v1, v2, v3;
        v0.Position = glm::vec3(-width / 2.0f, 0.0f, -depth / 2.0f); // Bottom-left
        v1.Position = glm::vec3(width / 2.0f, 0.0f, -depth / 2.0f);  // Bottom-right
        v2.Position = glm::vec3(width / 2.0f, 0.0f, depth / 2.0f);   // Top-right
        v3.Position = glm::vec3(-width / 2.0f, 0.0f, depth / 2.0f);  // Top-left

        // Set texture coordinates for full texture coverage.
        v0.TexCoords = glm::vec2(0.0f, 0.0f);
        v1.TexCoords = glm::vec2(1.0f, 0.0f);
        v2.TexCoords = glm::vec2(1.0f, 1.0f);
        v3.TexCoords = glm::vec2(0.0f, 1.0f);

        // Set upward-facing normals for lighting.
        v0.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        v1.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        v2.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        v3.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);

        // Define indices for two triangles forming the quad.
        std::vector<GLuint> indices = {0, 1, 2, 0, 2, 3};

        // Create and store the mesh.
        meshes.emplace_back(GL_TRIANGLES, shader, texturePath, vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
    }

    // Constructs a heightmap-based terrain model.
    Model(const std::string &heightmapPath, ShaderProgram shader, const std::string &texturePath,
          int width, int depth, float heightScale)
        : shader(shader), name("heightmap"), type(HEIGHTMAP),
          width(static_cast<float>(width - 1)), depth(static_cast<float>(depth - 1)),
          heightScale(heightScale)
    {
        // Load grayscale heightmap image.
        cv::Mat heightmap = cv::imread(heightmapPath, cv::IMREAD_GRAYSCALE);
        if (heightmap.empty())
        {
            std::cerr << "Error: Failed to load heightmap: " << heightmapPath << "\n";
            throw std::runtime_error("Heightmap loading failed");
        }

        // Resize heightmap to match specified dimensions.
        if (heightmap.cols != width || heightmap.rows != depth)
        {
            cv::resize(heightmap, heightmap, cv::Size(width, depth));
        }

        // Store normalized height data.
        heightData.resize(width * depth);
        for (int z = 0; z < depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                heightData[z * width + x] = heightmap.at<uchar>(z, x) / 255.0f;
            }
        }

        // Generate vertex grid for the heightmap.
        std::vector<Vertex> vertices;
        for (int z = 0; z < depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                Vertex v;
                float height = heightmap.at<uchar>(z, x) / 255.0f * heightScale;
                v.Position = glm::vec3(x - width / 2.0f, height, z - depth / 2.0f);
                v.TexCoords = glm::vec2(static_cast<float>(x) / (width - 1), static_cast<float>(z) / (depth - 1));
                v.Normal = glm::vec3(0.0f, 1.0f, 0.0f); // Placeholder normal (updated later if needed).
                vertices.push_back(v);
            }
        }

        // Generate indices for triangle strips.
        std::vector<GLuint> indices;
        for (int z = 0; z < depth - 1; ++z)
        {
            for (int x = 0; x < width - 1; ++x)
            {
                int topLeft = z * width + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * width + x;
                int bottomRight = bottomLeft + 1;

                // First triangle: topLeft -> bottomLeft -> topRight
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // Second triangle: topRight -> bottomLeft -> bottomRight
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        // Create and store the mesh.
        meshes.emplace_back(GL_TRIANGLES, shader, texturePath, vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
    }

    // Constructs a spherical model.
    Model(int segments, ShaderProgram shader, glm::vec3 color)
        : shader(shader), name("sphere")
    {
        // Generate vertices for a unit sphere.
        std::vector<Vertex> vertices;
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
                vert.Normal = vert.Position; // Normals are equal to positions for a unit sphere.
                vert.TexCoords = glm::vec2(j / (float)segments, i / (float)segments);
                vertices.push_back(vert);
            }
        }

        // Generate indices for triangle mesh.
        std::vector<GLuint> indices;
        for (int i = 0; i < segments; ++i)
        {
            for (int j = 0; j < segments; ++j)
            {
                int first = i * (segments + 1) + j;
                int second = first + segments + 1;

                // First triangle
                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                // Second triangle
                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        // Create mesh with a yellow texture ("NONE") and set material colors.
        meshes.emplace_back(GL_TRIANGLES, shader, "NONE", vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
        meshes.back().diffuse_material = glm::vec4(color, 1.0f);
        meshes.back().ambient_material = glm::vec4(color, 1.0f);
        meshes.back().specular_material = glm::vec4(1.0f);
    }

    // Samples the height at a given world position for heightmap models.
    float getHeightAt(float worldX, float worldZ) const
    {
        if (type != HEIGHTMAP)
            return 0.0f;

        // Convert world coordinates to local UV coordinates.
        float localX = worldX - origin.x;
        float localZ = worldZ - origin.z;
        float u = (localX / width) + 0.5f;
        float v = (localZ / depth) + 0.5f;
        u = glm::clamp(u, 0.0f, 1.0f);
        v = glm::clamp(v, 0.0f, 1.0f);

        // Map to height data indices.
        float xPos = u * width;
        float zPos = v * depth;
        int x0 = static_cast<int>(xPos);
        int x1 = x0 + 1;
        int z0 = static_cast<int>(zPos);
        int z1 = z0 + 1;

        // Clamp indices to valid range.
        x0 = glm::clamp(x0, 0, static_cast<int>(width) - 1);
        x1 = glm::clamp(x1, 0, static_cast<int>(width) - 1);
        z0 = glm::clamp(z0, 0, static_cast<int>(depth) - 1);
        z1 = glm::clamp(z1, 0, static_cast<int>(depth) - 1);

        // Perform bilinear interpolation of height values.
        float h00 = heightData[z0 * width + x0];
        float h01 = heightData[z1 * width + x0];
        float h10 = heightData[z0 * width + x1];
        float h11 = heightData[z1 * width + x1];
        float xFactor = xPos - x0;
        float zFactor = zPos - z0;
        float top = h00 * (1 - xFactor) + h10 * xFactor;
        float bottom = h01 * (1 - xFactor) + h11 * xFactor;
        return (top * (1 - zFactor) + bottom * zFactor) * heightScale;
    }

    // Calculates the normal vector at a given world position for heightmap models.
    glm::vec3 getNormalAt(float worldX, float worldZ) const
    {
        if (type != HEIGHTMAP)
            return glm::vec3(0.0f, 1.0f, 0.0f);

        // Estimate normal using finite differences.
        const float epsilon = 0.1f;
        float height = getHeightAt(worldX, worldZ);
        float dx = getHeightAt(worldX + epsilon, worldZ) - height;
        float dz = getHeightAt(worldX, worldZ + epsilon) - height;

        // Compute normal via cross product of tangent and bitangent.
        glm::vec3 tangent(1.0f, dx / epsilon, 0.0f);
        glm::vec3 bitangent(0.0f, dz / epsilon, 1.0f);
        return glm::normalize(glm::cross(tangent, bitangent));
    }

    // Updates model transformations based on elapsed time.
    void update(const float totalTime)
    {
        if (name == "cube")
        {
            // Apply continuous rotation for cube models (disabled for now).
            // orientation.x = fmod(totalTime * 30.0f, 360.0f); // 30°/sec around X
            // orientation.y = fmod(totalTime * 45.0f, 360.0f); // 45°/sec around Y
            // orientation.z = fmod(totalTime * 60.0f, 360.0f); // 60°/sec around Z
        }
        else if (name != "heightmap")
        {
            // Apply Y-axis rotation for non-heightmap models (disabled for now).
            // orientation.y = fmod(totalTime * 45.0f, 360.0f);
        }
    }

    // Renders all meshes in the model with specified transformations.
    void draw(glm::vec3 const &offset = glm::vec3(0.0f), glm::vec3 const &rotation = glm::vec3(0.0f))
    {
        for (const auto &mesh : meshes)
        {
            mesh.draw(origin + offset, orientation + rotation, isSun);
        }
    }
};
