//
// WARNING:
// In general, you can NOT freely reorder includes!
//

// C++
// include anywhere, in any order
#include <iostream>
#include <fstream>
#include <chrono>
#include <stack>
#include <random>
#include <string>

// OpenCV (does not depend on GL)
#include <opencv2\opencv.hpp>

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h> 
// WGLEW = Windows GL Extension Wrangler (change for different platform) 
// platform specific functions (in this case Windows)
#include <GL/wglew.h> 

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.
#include <GLFW/glfw3.h>

// OpenGL math (and other additional GL libraries, at the end)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <nlohmann/json.hpp> // Include JSON library

#include "app.hpp"
#include "gl_err_callback.h"

using json = nlohmann::json; // Alias for convenience

App::App()
{
	// default constructor
	// nothing to do here (so far...)
	std::cout << "Constructed...\n";

}

bool App::init()
{
	try {
		// initialization code
		//...
		// init glfw
		// https://www.glfw.org/documentation.html
		glfwInit();

		glfwSetErrorCallback(error_callback);

		// Load settings from app_settings.json
		std::ifstream settingsFile("app_settings.json");
		if (settingsFile.is_open()) {
			try {
				json settings = json::parse(settingsFile);

				// Set appname (with fallback)
				if (settings.contains("appname") && settings["appname"].is_string()) {
					appname = settings["appname"].get<std::string>();
				}

				// Set default_resolution (with fallback)
				if (settings.contains("default_resolution") && settings["default_resolution"].is_object()) {
					if (settings["default_resolution"].contains("x") && settings["default_resolution"]["x"].is_number_integer()) {
						resX = settings["default_resolution"]["x"].get<int>();
					}
					if (settings["default_resolution"].contains("y") && settings["default_resolution"]["y"].is_number_integer()) {
						resY = settings["default_resolution"]["y"].get<int>();
					}
				}
			}
			catch (const json::exception& e) {
				std::cerr << "JSON parsing error: " << e.what() << '\n';
				// Continue with defaults
			}
			settingsFile.close();
		}
		else {
			std::cerr << "Could not open app_settings.json, using defaults\n";
		}

		// Explicitly request OpenGL 4.6 Compatibility Profile (default-like)
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // Try CORE_PROFILE if needed

		// open window (GL canvas) with no special properties
		// https://www.glfw.org/docs/latest/quick.html#quick_create_window
		window = glfwCreateWindow(resX, resY, appname.c_str(), NULL, NULL);
		glfwMakeContextCurrent(window);
		glfwSetWindowUserPointer(window, this);
		glfwSetScrollCallback(window, [](GLFWwindow* w, double x, double y) {
			static_cast<App*>(glfwGetWindowUserPointer(w))->scroll_callback(w, x, y);
		});
		glfwSetKeyCallback(window, [](GLFWwindow* w, int k, int s, int a, int m) {
			static_cast<App*>(glfwGetWindowUserPointer(w))->key_callback(w, k, s, a, m);
		});

		// init glew
		// http://glew.sourceforge.net/basic.html
		glewInit();
		wglewInit();
		// if (not_success)
		//  throw std::runtime_error("something went bad");

		// HOWTO get string safely ( glGetString() may return nullptr ) 
		// Get OpenGL version
		const char* openGLVersion = (const char*)glGetString(GL_VERSION);
		if (openGLVersion == nullptr)
			std::cout << "<Unknown>\n";
		else
			std::cout << "Value is:" << openGLVersion << '\n';

		// HOWTO get integer
		// Get profile info with debugging
		GLint myint;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &myint);

		if (myint & GL_CONTEXT_CORE_PROFILE_BIT) {
			std::cout << "We are using CORE profile\n";
		}
		else {
			if (myint & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
				std::cout << "We are using COMPATIBILITY profile\n";
			}
			else {
				throw std::runtime_error("What??");
			}
		}

		if (GLEW_ARB_debug_output)
		{
			glDebugMessageCallback(MessageCallback, 0);
			glEnable(GL_DEBUG_OUTPUT);

			//default is asynchronous debug output, use this to simulate glGetError() functionality
			//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

			std::cout << "GL_DEBUG enabled." << std::endl;
		}
		else
			std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;

		// Set initial VSync state
		glfwSwapInterval(vsyncEnabled ? 1 : 0);
	}
	catch (std::exception const& e) {
		std::cerr << "Init failed : " << e.what() << std::endl;
		throw;
	}
	std::cout << "Initialized...\n";

	return true;
}

int App::run(void)
{
	if (!window) return -1;

	double lastTime = glfwGetTime(); // Time of last FPS update
	int frameCount = 0; // Number of frames since last update

	try {
		// app code
		//...

		while (!glfwWindowShouldClose(window))
		{
			// Measure time
			double currentTime = glfwGetTime();
			frameCount++;

			// Update FPS every second
			if (currentTime - lastTime >= 1.0) {
				double fps = frameCount / (currentTime - lastTime);
				std::string title = "FPS: " + std::to_string(static_cast<int>(fps + 0.5)) +
					" | VSync: " + (vsyncEnabled ? "On" : "Off");
				glfwSetWindowTitle(window, title.c_str());

				frameCount = 0;
				lastTime = currentTime;
			}

			// Clear OpenGL canvas, both color buffer and Z-buffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Swap front and back buffers
			glfwSwapBuffers(window);

			// Poll for and process events
			glfwPollEvents();
		}
	}
	catch (std::exception const& e) {
		std::cerr << "App failed : " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Finished OK...\n";
	return EXIT_SUCCESS;
}

void App::error_callback(int error, const char* description) {
	std::cerr << "GLFW Error " << error << ": " << description << '\n';
}


void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	std::cout << "Scroll: " << xoffset << ", " << yoffset << '\n';
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_F12: // Changed from GLFW_KEY_V to GLFW_KEY_F12
			vsyncEnabled = !vsyncEnabled; // Toggle VSync state
			glfwSwapInterval(vsyncEnabled ? 1 : 0); // Apply VSync setting
			// Title will update in the next FPS cycle
			break;
		default:
			break;
		}
	}
}


App::~App()
{
	// clean-up
	if (window)
		glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
	cv::destroyAllWindows();
	std::cout << "Bye...\n";
}
