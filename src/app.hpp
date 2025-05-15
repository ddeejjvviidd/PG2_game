#pragma once

#include <GL/glew.h>    // Add this to ensure GLEW comes first
#include "assets.hpp"   // Already includes glew.h, but we make it explicit
#include <GLFW/glfw3.h> // GLFW comes after GLEW
#include "camera.hpp"
#include <glm/glm.hpp>
#include "Model.hpp"
#include <string>
#include <vector>

// class App
// {
// public:
//     App();

//     bool init(void);
//     void init_assets(void);
//     int run(void);
//     static void error_callback(int error, const char *description);
//     void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
//     void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
//     void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

//     GLuint shader_prog_ID;
//     GLuint VBO_ID{0};
//     GLuint VAO_ID{0};

//     GLfloat r{1.0f}, g{0.0f}, b{0.0f}, a{1.0f};

//     Mesh mesh;

//     ~App();

// private:
//     GLFWwindow *window = nullptr;
//     bool vsyncEnabled;   // Tracks VSync state
//     std::string appname; // Stores "first_test"
//     int resX;            // Stores default_resolution.x (1024)
//     int resY;            // Stores default_resolution.y (768)
//     Model model;         // Manages the object
// };

class App
{
public:
    App();
    ~App();
    bool init();
    void init_assets();
    int run();

    // Callbacks
    static void error_callback(int error, const char *description);
    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
    bool checkFloorCollision(const glm::vec3 &position, float playerHalfHeight, float &floorHeight);
    void toggleFullscreen();

    struct DirectionalLight
    {
        glm::vec3 direction;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;

        DirectionalLight()
            : direction(glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f))),
              ambient(glm::vec3(0.2f)),
              diffuse(glm::vec3(0.5f)),
              specular(glm::vec3(1.0f)) {}
    };

    struct PointLight
    {
        glm::vec3 position;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float constant;
        float linear;
        float quadratic;

        PointLight() : position(0.0f),
                       ambient(0.1f, 0.1f, 0.1f),
                       diffuse(0.8f, 0.8f, 0.8f),
                       specular(1.0f, 1.0f, 1.0f),
                       constant(1.0f),
                       linear(0.09f),
                       quadratic(0.032f) {}
    };

    struct SpotLight
    {
        glm::vec3 position;
        glm::vec3 direction;
        float cutOff;
        float outerCutOff;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;

        SpotLight() : position(0.0f),
                      direction(0.0f, 0.0f, 1.0f),
                      cutOff(glm::cos(glm::radians(12.5f))),
                      outerCutOff(glm::cos(glm::radians(17.5f))),
                      ambient(0.0f, 0.0f, 0.0f),
                      diffuse(1.0f, 1.0f, 1.0f),
                      specular(1.0f, 1.0f, 1.0f) {}
    };

    DirectionalLight sun; // Add sun as a member

    size_t sunModelIndex;

private:
    GLFWwindow *window;
    GLuint shader_prog_ID;
    std::vector<Model> models;
    std::vector<Model> floor;
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    bool vsyncEnabled = true;
    bool antiAliasingEnabled = false; // default value
    int antiAliasingSamples = 2;      // default value

    double startTime = glfwGetTime();
    double lastFrameTime = startTime;
    double lastFpsUpdate = startTime;
    int frameCount = 0;

    // Transformation-related members
    glm::mat4 projectionMatrix;
    float fov = 60.0f;
    int windowWidth = 800;
    int windowHeight = 600;

    // Camera-related members
    Camera camera{glm::vec3(0.0f, 0.0f, 1.0f)}; // Start 1 unit back
    double cursorLastX = 0.0;
    double cursorLastY = 0.0;

    std::string appname; // Stores "first_test"
    int resX;            // Stores default_resolution.x (1024)
    int resY;            // Stores default_resolution.y (768)

    PointLight pointLights[3]; // Array for 3 point lights
    SpotLight spotLight;       // Single spot light
    bool spotLightEnabled = true;

    void UpdateLightUniforms(ShaderProgram &shader);

    bool isFullscreen = false;
    int windowPosX = 100, windowPosY = 100;        // default starting position
    int windowedWidth = 800, windowedHeight = 600; // default windowed size
};