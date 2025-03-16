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
    void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

private:
    GLFWwindow *window;
    GLuint shader_prog_ID;
    Model model;
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    bool vsyncEnabled = true;

    // Transformation-related members
    glm::mat4 projectionMatrix;
    float fov = 60.0f;
    int windowWidth = 800;
    int windowHeight = 600;

    // Camera-related members
    Camera camera{glm::vec3(0.0f, 0.0f, 5.0f)}; // Start 5 units back
    double cursorLastX = 0.0;
    double cursorLastY = 0.0;

    std::string appname; // Stores "first_test"
    int resX;            // Stores default_resolution.x (1024)
    int resY;            // Stores default_resolution.y (768)
};