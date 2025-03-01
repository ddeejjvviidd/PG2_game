#pragma once

#include <GLFW/glfw3.h>
#include <string>

class App
{
public:
    App();

    bool init(void);
    int run(void);
    static void error_callback(int error, const char *description);
    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    ~App();

private:
    GLFWwindow *window = nullptr;
    bool vsyncEnabled;   // Tracks VSync state
    std::string appname; // Stores "first_test"
    int resX;            // Stores default_resolution.x (1024)
    int resY;            // Stores default_resolution.y (768)
};
