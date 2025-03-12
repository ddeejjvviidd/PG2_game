#pragma once

#include <filesystem>
#include <string>
#include <vector> 
#include <glm/glm.hpp> 

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"

class Model {
public:
    std::vector<Mesh> meshes;
    std::string name;
    glm::vec3 origin{};
    glm::vec3 orientation{};
    ShaderProgram shader{0};
    
    Model(const std::filesystem::path & filename, ShaderProgram shader) {
        // load mesh (all meshes) of the model, (in the future: load material of each mesh, load textures...)
        // TODO: call LoadOBJFile, LoadMTLFile (if exist), process data, create mesh and set its properties
        //    notice: you can load multiple meshes and place them to proper positions, 
        //            multiple textures (with reusing) etc. to construct single complicated Model   
    }

    // update position etc. based on running time
    void update(const float delta_t) {
        // origin += glm::vec3(3,0,0) * delta_t; // s = s0 + v*dt
    }
    
    void draw(glm::vec3 const & offset = glm::vec3(0.0), glm::vec3 const & rotation = glm::vec3(0.0f)) {
        // call draw() on mesh (all meshes)
        for (auto const& mesh : meshes) {
            mesh.draw(origin+offset, orientation+rotation);
        }
    }
};

